from __future__ import annotations

import os
from enum import Enum, auto
from ast import literal_eval
from argparse import ArgumentParser
from itertools import chain



def _print_lineinfo(linfo):
    lineno, txt = linfo
    print(f'  In line #{lineno}: "{txt}"')

def warning(notif: Notif, *args, **kwargs):
    print('~ WARNING:', *args, **kwargs)
    _print_lineinfo(notif.lineinfo)

def fatal_error(notif: Notif, *args, **kwargs):
    print('- ERROR:', *args, **kwargs)
    _print_lineinfo(notif.lineinfo)
    exit(1)

def warning_nonotif(*args, **kwargs):
    print('~ WARNING:', *args, **kwargs)

def fatal_nonotif(*args, **kwargs):
    print('- ERROR:', *args, **kwargs)
    exit(1)

def wfp_closures(notif: Notif, prefix: str):
    def warn(*args, **kwargs):
        warning(notif, prefix, *args, **kwargs)
    def fatal(*args, **kwargs):
        fatal_error(notif, prefix, *args, **kwargs)
    def good(*args, **kwargs):
        approve(prefix, *args, **kwargs)
    return warn, fatal, good

# Exists in 3.9, but 3.8 on ineks. Have to add myself. Some code is straight from the PEP!
def rmpref(self: str, prefix: str, /) -> str:
    if self.startswith(prefix):
        return self[len(prefix):]
    else:
        return self[:]

def rmsuff(self: str, suffix: str, /) -> str:
    # suffix='' should not call self[:-0].
    if suffix and self.endswith(suffix):
        return self[:-len(suffix)]
    else:
        return self[:]

class ExtStr:
    def __init__(self, s):
        self.s = s

    def rmpref(self, prefix):
        return ExtStr(rmpref(self.s, prefix))

    def rmsuff(self, suffix):
        return ExtStr(rmsuff(self.s, suffix))

    def unbox(self):
        return self.s

def box(s):
    return ExtStr(s)

class Action(Enum):
    PPCREATE = auto()
    PPARRIVE = auto()
    PPGATHER = auto()
    PPCLEAR = auto()
    PPEXIT = auto()
    PPBREAK = auto()
    PPSTOP = auto()
    PPCONT = auto()
    OBREAK = auto()
    OCONT = auto()
    OSTOP = auto()
    SSCREATE = auto()
    SSARRIVE = auto()
    SSFLICK = auto()
    SSLEFT = auto()
    SSEXIT = auto()
    SSSTOP = auto()

class Notif:
    def __init__(self, lineno, line, time, tid, action, aid=None, i=None, j=None):
        self.lineinfo = (lineno, line.strip())
        self.time = time
        self.tid = tid
        self.action = action
        self.aid = aid
        self.i = i
        self.j = j

    def __repr__(self):
        pref = f't={self.time}µs, tid={self.tid}, action={self.action}'
        if self.aid is None:
            return pref
        else:
            pref = f'{pref}, id={self.aid}'
            if self.i is None and self.j is None:
                return pref
            else:
                return f'{pref}, i={self.i}, j={self.j}'

class NotifType(Enum):
    PP = auto()
    ORDER = auto()
    SS = auto()

def notif_type(notif: Notif):
    a = notif.action
    if a in (Action.OCONT, Action.OSTOP, Action.OBREAK):
        return NotifType.ORDER
    elif a in (Action.SSCREATE, Action.SSARRIVE, Action.SSFLICK, Action.SSLEFT, Action.SSEXIT,
               Action.SSEXIT, Action.SSSTOP):
        return NotifType.SS
    else:
        return NotifType.PP

def _parse_coord_from_end(txt):
    *_, l1, l2 = txt.strip().split(' ')
    l2 = rmsuff(l2, '.')
    try:
        return literal_eval(l2)
    except SyntaxError:
        return literal_eval(l1 + l2)

def parse_action(txt):
    if txt == 'BREAK!':
        return Action.OBREAK
    elif txt == 'CONTINUE!':
        return Action.OCONT
    elif txt == 'STOP!':
        return Action.OSTOP
    else: 
        is_g = txt.startswith('G')
        if not is_g and not txt.startswith('S'):
            raise ValueError(f'Notification tail should start with S or G: "{txt}"')
        aidstr, rest = txt.split(' ', 1)
        aid = int(aidstr[1:])
        rest = rest.strip()
        if rest == 'created.':
            a = Action.PPCREATE if is_g else Action.SSCREATE
            return a, aid
        elif rest == 'cleared the current area and left.':
            assert is_g
            return Action.PPCLEAR, aid
        elif rest == 'finished cleaning and exited.':
            assert is_g
            return Action.PPEXIT, aid
        elif rest == 'took a break.':
            assert is_g
            return Action.PPBREAK, aid
        elif rest == 'stopped as ordered.':
            a = Action.PPSTOP if is_g else Action.SSSTOP
            return a, aid
        elif rest == 'is continuing after a break.':
            assert is_g
            return Action.PPCONT, aid
        elif rest == 'had enough of the current area and left.':
            assert not is_g
            return Action.SSLEFT, aid
        elif rest == 'finished smoking and exited.':
            assert not is_g
            return Action.SSEXIT, aid
        else:
            i, j = _parse_coord_from_end(rest)
            if rest.startswith('arrived at area'):
                assert is_g
                a = Action.PPARRIVE
            elif rest.startswith('arrived at cell'):
                assert not is_g
                a = Action.SSARRIVE
            elif rest.startswith('gathered a cigbutt from'):
                assert is_g
                a = Action.PPGATHER
            elif rest.startswith('flicked a cigbutt towards'):
                assert not is_g
                a = Action.SSFLICK
            else:
                raise ValueError(f'Unknown notification tail: {rest}')
            return a, aid, i, j

def wrap4(action):
    if isinstance(action, tuple):
        if len(action) == 4:
            return action
        elif len(action) == 2:
            return *action, None, None
        else:
            raise ValueError(f'Unexpected length {len(action)}!')
    else:
        return action, None, None, None

def parse_notification(txt: str, lineno: int):
    try:
        tstr, rest = txt.strip().split('|')
        time = int(rmpref(tstr, 't:').strip())
        tidstr, rest = rest.strip().split(' ', 1)
        tid = int(box(tidstr.strip()).rmpref('(T').rmsuff(')').unbox())
        return Notif(lineno, txt, time, tid, *wrap4(parse_action(rest)))
    except:
        raise ValueError(f'Could not parse notification: "{txt}"')

def parse_notifications_from(file_path):
    with open(file_path, 'r') as f:
        for i, line in enumerate(f, 1):
            yield parse_notification(line, i)




# Penalties implemented with a global dictionary to avoid repeating the penalty
class Penalty(Enum):
    OTHER_NOTIFS_AFTER_ORDERS = auto()
    REACTING_TO_POINTLESS_ORDERS = auto()
    TOO_QUICK_ORDERS = auto()
    ORDER_LATE = auto()
    ORDER_REACTION_LATE = auto()
    GATHER_LATE = auto()
    GATHER_EARLY = auto()
    BLOCKED_FOR_NO_REASON_PT3 = auto()
    FLICK_LATE = auto()
    FLICK_EARLY = auto()

# Over 100
PENALTY_SCORES = {
    Penalty.OTHER_NOTIFS_AFTER_ORDERS: 20,
    Penalty.REACTING_TO_POINTLESS_ORDERS: 10,
    Penalty.TOO_QUICK_ORDERS: 15,
    Penalty.ORDER_LATE: 20,
    Penalty.ORDER_REACTION_LATE: 10,
    Penalty.GATHER_LATE: 10,
    Penalty.GATHER_EARLY: 35,
    Penalty.BLOCKED_FOR_NO_REASON_PT3: 50,
    Penalty.FLICK_LATE: 15, # slightly harsher for pt3
    Penalty.FLICK_EARLY: 40,
}

g_applied_penalties = {}

def g_penalize(penalty_type: Penalty):
    global g_applied_penalties
    print(f'  Penalized with: {penalty_type.name} (applied only once)')
    g_applied_penalties[penalty_type] = PENALTY_SCORES[penalty_type]

def g_tally_penalties():
    penalty = 0
    if g_applied_penalties:
        print('Applied penalties:')
        for k, v in g_applied_penalties.items():
            print(f'* {k.name}: {v} points')
            penalty += v
        print(f'=> Total: {penalty} points')
        print()
    penalty = min(penalty, 100)
    return penalty




def order2str(a: Action):
    if a == Action.OCONT:
        return 'CONTINUE'
    elif a == Action.OBREAK:
        return 'BREAK'
    elif a == Action.OSTOP:
        return 'STOP'
    else:
        raise ValueError(f'{a} is not an order action.')

class PP:
    def __init__(self, aid: int, si: int, sj: int, gt: int, areas: list):
        self.aid = aid
        self.tid = None
        self.last_action = None
        self.si = si
        self.sj = sj
        self.gt = gt
        self.areas = areas
        self.area_idx = 0
        self.in_area = False
        self.in_break = False
        self.exited = False
        self.cur_cell = None
        self.next_notif_should_react_to = None
        self.last_gather_time = None
        self.last_leave_time = 0
        self.lock_is_late = False

    def _update_cell(self, state):
        start_i, start_j = self.areas[self.area_idx]
        cur_i, cur_j = self.cur_cell
        while cur_i < start_i + self.si and state.get(cur_i, cur_j) == 0:
            cur_j += 1
            if start_j + self.sj == cur_j:
                cur_i += 1
                cur_j = start_j
        self.cur_cell = (cur_i, cur_j)

    def has_to_react(self):
        return self.next_notif_should_react_to is not None

    def exit_check(self):
        if not self.exited:
            fatal_nonotif(f'PP{self.aid} has not exited at the end of the notification stream. '
                           'Notifications are incomplete!')

    def apply(self, notif: Notif, state: State):
        warn, fatal, good = wfp_closures(notif, f'PP{self.aid}')
        nt = notif_type(notif)

        if nt == NotifType.ORDER:
            if self.exited:
                return
            elif state.active_order != notif.action:
                if self.next_notif_should_react_to is not None:
                    # got another order before having reacted to this one
                    warn(f'Got order for {order2str(notif.action)}, while still having to react to'
                         f' {order2str(self.next_notif_should_react_to)}. Overriding with new order'
                          ', but orders are too quick or reactions are bad. This may cause a fatal '
                          'error down the line.')
                    g_penalize(Penalty.TOO_QUICK_ORDERS)
                self.next_notif_should_react_to = notif.action
            return
        
        if self.exited:
            fatal(f'still has notifs after exiting/stopping.')

        if notif_type(notif) != NotifType.PP or notif.aid != self.aid:
            return

        if self.tid is None:
            assert self.last_action is None
            if notif.action != Action.PPCREATE:
                fatal(f'first notif should be creation!')
            good(f'created with tid={notif.tid}.')
            self.tid = notif.tid
        else:
            self._tid_check(notif)
            a = notif.action

            reacting_to = self.next_notif_should_react_to
            if reacting_to is not None:
                reacted_ok = False
                if reacting_to == Action.OBREAK:
                    if a == Action.PPBREAK:
                        reacted_ok = True
                        good('took a break.')
                        self.in_break = True
                        # also unlock the area during a break
                        if self.in_area:
                            cur_area = self.areas[self.area_idx]
                            state.ppunlock(self.aid, *cur_area, self.si, self.sj)
                            self.in_area = False
                    else:
                        warn('had to take a break after the order, but doing something else.')
                        g_penalize(Penalty.OTHER_NOTIFS_AFTER_ORDERS)
                elif reacting_to == Action.OCONT:
                    if a == Action.PPCONT:
                        reacted_ok = True
                        good('continued after a break.')
                        self.in_break = False
                        self.last_leave_time = notif.time // 1000
                    else:
                        warn('had to continue after the order, but doing something else.')
                        g_penalize(Penalty.OTHER_NOTIFS_AFTER_ORDERS)
                elif reacting_to == Action.OSTOP:
                    if a == Action.PPSTOP:
                        reacted_ok = True
                        self.exited = True
                        good('stopped as ordered.')
                    else:
                        warn('had to stop after the order, but doing something else.')
                        g_penalize(Penalty.OTHER_NOTIFS_AFTER_ORDERS)
                else:
                    raise RuntimeError('Problem in checker, should not be reacting to !order.')
                        
                # if reacted properly, no need to react next time, good
                # also check the reaction time
                if reacted_ok:
                    diff = (notif.time - state.active_order_given_time) // 1000
                    if diff > state.ort:
                        warn(f'order reaction is late. Reacted to order {diff} ms after order '
                             f'was given, but tolerance is {state.ort} ms.')
                        g_penalize(Penalty.ORDER_REACTION_LATE)
                    elif diff < 0:
                        raise RuntimeError('Reaction before order? Logic error in checker.')
                    self.next_notif_should_react_to = None
                    return
                # otherwise, need to process the notification and still need to react later

            if self.in_break and a != Action.PPCONT and a != Action.PPSTOP:
                fatal(f'sent a notification while still in break.')
           
            # if we see an order reaction here, it means it happened when there is nothing
            # to react to. either a repeated order, or sending order notifs for no reason.
            if a == Action.PPBREAK:
                if self.in_break:
                    warn('took a break while already in a break.')
                    g_penalize(Penalty.REACTING_TO_POINTLESS_ORDERS)
                else:
                    fatal('took a break with no order to do so!')
            elif a == Action.PPCONT:
                if not self.in_break:
                    warn('continued while already not in a break.')
                    g_penalize(Penalty.REACTING_TO_POINTLESS_ORDERS)
                else:
                    fatal('continued with no order to do so!')
            elif a == Action.PPSTOP:
                fatal('stopped with no order to do so!')
            elif a == Action.PPCREATE:
                fatal(f'sent a second creation notification. Nope.')
            elif a == Action.PPARRIVE:
                cur_area = self.areas[self.area_idx]
                notif_area = (notif.i, notif.j)
                if cur_area != notif_area:
                    fatal(f'arriving at {notif_area}, but should have '
                          f'arrived at {cur_area}.')
                if self.in_area:
                    fatal(f'arrived at {notif_area} before leaving previous '
                          f'area {cur_area}.')
                # will check for locks
                attempt = state.pplock(self.aid, notif.i, notif.j, self.si, self.sj)
                if attempt is not None:
                    by_pp, by_aid, (li, lj, lsi, lsj) = attempt
                    fatal(f'tried to lock area at {notif_area} size {self.si}x{self.sj} ' +
                          f'that intersects with area ({li}, {lj}) sized {lsi}x{lsj} currently ' +
                          (f'locked by PP{by_aid}.' if by_pp else f'used by SS{by_aid}.'))
                self.in_area = True
                self.cur_cell = cur_area
                self.last_gather_time = notif.time
                self._update_cell(state)
                self.lock_is_late = False # reset lateness since new area entered
                good(f'arrived at area {notif_area}.')
            elif a == Action.PPCLEAR:
                if not self.in_area:
                    fatal(f'cleared area before having arrived in an area.')
                cur_area = self.areas[self.area_idx]
                state.ppunlock(self.aid, *cur_area, self.si, self.sj)
                cigcount = state.count_cigs(*cur_area, self.si, self.sj)
                if cigcount > 0:
                    fatal(f'cleared area before all cigbutts were gathered, {cigcount} left.')
                self.in_area = False
                self.area_idx += 1
                self.last_leave_time = notif.time // 1000
                good(f'left last area, which was {cur_area}.')
            elif a == Action.PPEXIT:
                if self.area_idx < len(self.areas):
                    fatal(f'exited before all areas are cleared.')
                self.exited = True
                good('exited properly.')
            elif a == Action.PPGATHER:
                # position check
                notif_cell = (notif.i, notif.j)
                if notif_cell != self.cur_cell:
                    fatal(f'gathering from cell {notif_cell}, expected {self.cur_cell} in order.')

                # time check
                if self.last_gather_time is not None:
                    diff = (notif.time - self.last_gather_time) // 1000
                    if diff > state.gdt + self.gt:
                        warn(f'gathering is late. Expected {self.gt} ms between gatherings, '
                             f'but got {diff} ms, beyond {state.gdt} ms tolerance.')
                        g_penalize(Penalty.GATHER_LATE)
                    elif diff < self.gt - state.gdt:
                        warn(f'expected {self.gt} ms between gatherings, but got only '
                             f'{diff} ms. Too early! Why? Significant problem since '
                              'it goes against homework constraints.')
                        g_penalize(Penalty.GATHER_EARLY)
                self.last_gather_time = notif.time

                # count check
                value = state.decr(*notif_cell)
                if value < 0:
                    fatal(f'decremented cigbutt count in cell {notif_cell} below zero.')
                elif value == 0:
                    self._update_cell(state)
                good(f'gathered from {notif_cell}, leaving {value} cigbutts in the cell.')
            else:
                raise ValueError(f'Unexpected action {a}')
        self.last_action = notif.action

    def lock_delay_check(self, current_ms: int, state: State):
        if (not self.exited and not self.in_break and not self.in_area and 
                not self.lock_is_late and self.area_idx < len(self.areas)):
            # first, cancel if delay is small to avoid block checking
            if state.ppblocked(*self.areas[self.area_idx], self.si, self.sj):
                # update last leave time, since practically blocked
                self.last_leave_time = current_ms
            else:
                delay = current_ms - self.last_leave_time
                if delay > state.ldt:
                    # otherwise late
                    self.lock_is_late = True # prevent re-checks
                    if self.last_leave_time == 0:
                        warning_nonotif(f'PP{self.aid} still has not locked its first '
                                        f'area after {delay} ms, even though it is not '
                                        'blocked.')
                    else:
                        warning_nonotif(f'PP{self.aid} still has not locked its next area '
                                        f'{delay} ms after leaving its last area, even '
                                        'though it is not blocked.')
                    g_penalize(Penalty.BLOCKED_FOR_NO_REASON_PT3)

    def _tid_check(self, notif):
        warn, fatal, _ = wfp_closures(notif, f'PP{self.aid}')
        if self.tid != notif.tid:
            if notif.action == Action.PPSTOP:
                warn(f'Another thread with tid={notif.tid} sent PP{self.aid} stop. Not ideal.')
            elif notif.action == Action.PPEXIT:
                warn(f'Another thread with tid={notif.tid} sent PP{self.aid} exit. Not ideal.')
            elif self.last_action == Action.PPCREATE:
                warn(f'PP created by tid={self.tid}, but got first notif from '
                     f'tid={notif.tid}. Updating to use {notif.tid}. Not ideal.')
                self.tid = notif.tid
            else:
                fatal(f'tid changed for PP{self.aid} between actions! Unacceptable. \n'
                      f'previous action: {self.last_action}, tid={self.tid}\n'
                      f'current action: {notif.action}, tid={notif.tid}')

def intersect_1d(s1, e1, s2, e2):
    return s2 <= e1 and s1 <= e2

def intersect_2d(a1, a2):
    i1, j1, si1, sj1 = a1
    i2, j2, si2, sj2 = a2
    return intersect_1d(i1, i1 + si1 - 1, i2, i2 + si2 - 1) and \
           intersect_1d(j1, j1 + sj1 - 1, j2, j2 + sj2 - 1)

class SS:
    OFFSETS = [(-1, -1), (-1, 0), (-1, 1), (0, 1), (1, 1), (1, 0), (1, -1), (0, -1)]

    def __init__(self, aid: int, ft: int, area_counts: list):
        self.aid = aid
        self.tid = None
        self.last_action = None
        self.ft = ft
        self.area_counts = area_counts
        self.area_idx = 0
        self.in_area = False
        self.exited = False
        self.cur_cell = None
        self.next_notif_should_react_to = None
        self.last_flick_time = None
        self.last_leave_time = 0
        self.lock_is_late = False
        self.off_idx = 0
        self.flicks_left = 0

    @property
    def target_cell(self):
        (center_i, center_j), _ = self.area_counts[self.area_idx]
        off_i, off_j = SS.OFFSETS[self.off_idx]
        return center_i + off_i, center_j + off_j

    def has_to_react(self):
        return self.next_notif_should_react_to is not None

    def exit_check(self):
        if not self.exited:
            fatal_nonotif(f'SS{self.aid} has not exited at the end of the notification stream. '
                           'Notifications are incomplete!')

    def apply(self, notif: Notif, state: State):
        warn, fatal, good = wfp_closures(notif, f'SS{self.aid}')
        nt = notif_type(notif)
        
        if nt == NotifType.ORDER:
            if not self.exited and notif.action == Action.OSTOP:
                self.next_notif_should_react_to = notif.action
            return
        
        if self.exited:
            fatal(f'still has notifs after exiting/stopping.')

        if nt != NotifType.SS or notif.aid != self.aid:
            return

        if self.tid is None:
            assert self.last_action is None
            if notif.action != Action.SSCREATE:
                fatal(f'first notif should be creation!')
            good(f'created with tid={notif.tid}.')
            self.tid = notif.tid
        else:
            self._tid_check(notif)
            a = notif.action

            reacting_to = self.next_notif_should_react_to
            if reacting_to is not None:
                if reacting_to == Action.OSTOP:
                    if a == Action.SSSTOP:
                        reacted_ok = True
                        self.exited = True
                        good('stopped as ordered.')
                    else:
                        warn('had to stop after the order, but doing something else.')
                        g_penalize(Penalty.OTHER_NOTIFS_AFTER_ORDERS)
                else:
                    raise RuntimeError('Problem in checker, should not be reacting to !stop.')
                        
                # if reacted properly, no need to react next time, good
                # also check the reaction time
                if reacted_ok:
                    diff = (notif.time - state.active_order_given_time) // 1000
                    if diff > state.ort:
                        warn(f'order reaction is late. Reacted to order {diff} ms after order '
                             f'was given, but tolerance is {state.ort} ms.')
                        g_penalize(Penalty.ORDER_REACTION_LATE)
                    elif diff < 0:
                        raise RuntimeError('Reaction before order? Logic error in checker.')
                    self.next_notif_should_react_to = None
                    return
                # otherwise, need to process the notification and still need to react later
           
            # if we see an order reaction here, it means it happened when there is nothing
            # to react to. either a repeated order, or sending order notifs for no reason.
            if a == Action.SSSTOP:
                fatal('stopped with no order to do so!')
            elif a == Action.SSCREATE:
                fatal(f'sent a second creation notification. Nope.')
            elif a == Action.SSARRIVE:
                cur_cell, self.flicks_left = self.area_counts[self.area_idx]
                notif_cell = (notif.i, notif.j)
                if cur_cell != notif_cell:
                    fatal(f'arriving at {notif_cell}, but should have '
                          f'arrived at {cur_cell}.')
                if self.in_area:
                    fatal(f'arrived at {notif_cell} before leaving previous '
                          f'cell {cur_cell}.')
                # will check for locks
                attempt = state.sslock(self.aid, *cur_cell)
                if attempt is not None:
                    *notif_area, _, _ = make_ss_area(*cur_cell)
                    notif_area = tuple(notif_area)
                    by_pp, by_aid, (li, lj, lsi, lsj) = attempt
                    fatal(f'tried to lock area at {notif_area} sized 3x3 ' +
                          f'that intersects with area ({li}, {lj}) sized {lsi}x{lsj} currently ' +
                          (f'locked by PP{by_aid}.' if by_pp else f'used by SS{by_aid}.'))
                self.in_area = True
                self.off_idx = 0
                self.last_flick_time = notif.time
                self.lock_is_late = False # reset lateness since new area entered
                good(f'arrived at cell {notif_cell}.')
            elif a == Action.SSLEFT:
                if not self.in_area:
                    fatal(f'left cell before having arrived in a cell.')
                cur_cell, _ = self.area_counts[self.area_idx]
                state.ssunlock(self.aid, *cur_cell)
                if self.flicks_left > 0:
                    fatal(f'left cell while still having {self.flicks_left} cigbutts to flick.')
                self.in_area = False
                self.area_idx += 1
                self.last_leave_time = notif.time // 1000
                good(f'left last cell, which was {cur_cell}.')
            elif a == Action.SSEXIT:
                if self.area_idx < len(self.area_counts):
                    fatal(f'exited before all areas were visited.')
                self.exited = True
                good('exited properly.')
            elif a == Action.SSFLICK:
                # position check
                notif_cell = (notif.i, notif.j)
                if notif_cell != self.target_cell:
                    fatal(f'flicking towards cell {notif_cell}, expected {self.target_cell} '
                          'in order.')

                # time check
                if self.last_flick_time is not None:
                    diff = (notif.time - self.last_flick_time) // 1000
                    if diff > state.gdt + self.ft:
                        warn(f'flicking is late. Expected {self.ft} ms between flicks, '
                             f'but got {diff} ms, beyond {state.gdt} ms tolerance.')
                        g_penalize(Penalty.FLICK_LATE)
                    elif diff < self.ft - state.gdt:
                        warn(f'expected {self.ft} ms between flicks, but got only '
                             f'{diff} ms. Too early! Why? Significant problem since '
                              'it goes against homework constraints.')
                        g_penalize(Penalty.FLICK_EARLY)
                self.last_flick_time = notif.time

                # incr count and change cell
                self.off_idx = (self.off_idx + 1) % len(SS.OFFSETS)
                value = state.incr(*notif_cell)
                self.flicks_left -= 1
                good(f'flicked towards {notif_cell}, now {value} cigbutts are in the cell.')
            else:
                raise ValueError(f'Unexpected action {a}')
        self.last_action = notif.action

    def lock_delay_check(self, current_ms: int, state: State):
        if (not self.exited and not self.in_area and 
                not self.lock_is_late and self.area_idx < len(self.area_counts)):
            cur_cell, _ = self.area_counts[self.area_idx]
            if state.ssblocked(*cur_cell):
                # update last leave time, since practically blocked
                self.last_leave_time = current_ms
            else:
                delay = current_ms - self.last_leave_time
                if delay > state.ldt:
                    # otherwise late
                    self.lock_is_late = True # prevent re-checks
                    if self.last_leave_time == 0:
                        warning_nonotif(f'SS{self.aid} still has not locked its first '
                                        f'cell after {delay} ms, even though it is not '
                                        'blocked.')
                    else:
                        warning_nonotif(f'SS{self.aid} still has not locked its next cell '
                                        f'{delay} ms after leaving its last cell, even '
                                        'though it is not blocked.')
                    g_penalize(Penalty.BLOCKED_FOR_NO_REASON_PT3)

    def _tid_check(self, notif):
        warn, fatal, _ = wfp_closures(notif, f'SS{self.aid}')
        if self.tid != notif.tid:
            if notif.action == Action.SSSTOP:
                warn(f'Another thread with tid={notif.tid} sent SS{self.aid} stop. Not ideal.')
            elif notif.action == Action.SSEXIT:
                warn(f'Another thread with tid={notif.tid} sent SS{self.aid} exit. Not ideal.')
            elif self.last_action == Action.SSCREATE:
                warn(f'SS created by tid={self.tid}, but got first notif from '
                     f'tid={notif.tid}. Updating to use {notif.tid}. Not ideal.')
                self.tid = notif.tid
            else:
                fatal(f'tid changed for SS{self.aid} between actions! Unacceptable. \n'
                      f'previous action: {self.last_action}, tid={self.tid}\n'
                      f'current action: {notif.action}, tid={notif.tid}')

def make_ss_area(ci, cj):
    return (ci - 1, cj - 1, 3, 3)

class State:
    def __init__(self, initial_grid, pps, orders, sss, odt, ort, gdt, ldt):
        self.pp_locked = []
        self.ss_locked = []
        self.grid = initial_grid
        self.pps = {pp.aid: pp for pp in pps}
        self.sss = {ss.aid: ss for ss in sss}
        self.orders = orders
        self.order_idx = 0
        self.active_order = Action.OCONT
        self.active_order_given_time = 0
        self.odt = odt # order delay tolerance
        self.ort = ort # order reaction delay tolerance
        self.gdt = gdt # gather delay tolerance
        self.ldt = ldt # lock delay tolerance, not used for part1 & 2
        self.ldcp = ldt // 20 # lock delay check sampling period
        self.ldc_counter = 0

    def apply(self, notif: Notif):
        nt = notif_type(notif)
        notif_ms = notif.time // 1000

        # handling the notif
        if nt == NotifType.PP:
            if notif.aid not in self.pps:
                fatal_error(notif, f'PP{notif.aid} should not exist.')
            self.pps[notif.aid].apply(notif, self)
        elif nt == NotifType.SS:
            if notif.aid not in self.sss:
                fatal_error(notif, f'SS{notif.aid} should not exist.')
            self.sss[notif.aid].apply(notif, self)
        elif nt == NotifType.ORDER:
            if self.order_idx >= len(self.orders):
                if len(self.orders) == 0:
                    fatal_error(notif, f'A {order2str(notif.action)} order was given, but there '
                                       'are no orders to give.')
                else:
                    fatal_error(notif, f'A {order2str(notif.action)} order was given, but there '
                                       'are no orders left to give.')
            target_ms, target_order = self.orders[self.order_idx]
            if order2str(notif.action) != target_order.name:
                fatal_error(notif, f'Expected {order2str(notif.action)} as the next order '
                                   f'but got {target_order.name}.')
            elif target_ms - notif_ms > self.odt:
                fatal_error(notif, f'Order is too early. Why? Expected at {target_ms} ms, '
                                   f'got at {notif_ms} ms. Beyond {self.odt} ms tolerance.')
            elif notif_ms - target_ms > self.odt:
                warning(notif, f'Order is late. Expected at {target_ms} ms, '
                               f'got at {notif_ms} ms. Beyond {self.odt} ms tolerance.')
                g_penalize(Penalty.ORDER_LATE)
            else:
                approve(f'Order {target_order.name} delivered at {notif_ms} ms, expected at '
                        f'{target_ms} ms. On time!')
            for soldier in chain(self.pps.values(), self.sss.values()):
                soldier.apply(notif, self)
            self.active_order = notif.action
            self.active_order_given_time = notif.time
            self.order_idx += 1
        else:
            raise ValueError()

        # checking for lock delays
        # ideal would be checking at every notification, but that slows down the checker
        # a lot. instead I use a sampling approach where I check for delays ever N ms.
        # This is not perfect and would be unfair for large N, but works well when N << ldt.
        if self.ldt > 0 and notif_ms > self.ldc_counter:
            for soldier in chain(self.pps.values(), self.sss.values()):
                soldier.lock_delay_check(notif_ms, self)
            self.ldc_counter += self.ldcp


    def exit_check(self):
        for soldier in chain(self.pps.values(), self.sss.values()):
            soldier.exit_check()
        if self.order_idx < len(self.orders):
            diff = len(self.orders) - self.order_idx
            fatal_nonotif('End of notification stream reached. Not all orders were delivered, '
                          f'the last {diff} orders remained.')
        approve('End of notification stream reached. Everyone has exited cleanly and all orders'
                ' were delivered. Nice!')

    def ppblocked(self, i, j, si, sj):
        cur_area = (i, j, si, sj)
        for locked, _ in self.pp_locked + self.ss_locked:
            if intersect_2d(locked, cur_area):
                return True
        return False

    def pplock(self, locker_id, i, j, si, sj):
        cur_area = (i, j, si, sj)
        for locked, aid in self.pp_locked:
            if intersect_2d(locked, cur_area):
                return True, aid, locked
        for locked, aid in self.ss_locked:
            if intersect_2d(locked, cur_area):
                return False, aid, locked
        self.pp_locked.append((cur_area, locker_id))
        return None

    def ppunlock(self, locker_id, i, j, si, sj):
        cur_area = (i, j, si, sj)
        self.pp_locked.remove((cur_area, locker_id))

    def ssblocked(self, ci, cj):
        cur_area = make_ss_area(ci, cj)
        for locked, _ in self.pp_locked:
            if intersect_2d(locked, cur_area):
                return True
        for locked, _ in self.ss_locked:
            if locked == cur_area:
                return True
        return False
    
    def sslock(self, locker_id, ci, cj):
        cur_area = make_ss_area(ci, cj)
        for locked, aid in self.pp_locked:
            if intersect_2d(locked, cur_area):
                return True, aid, locked
        for locked, aid in self.ss_locked:
            if locked == cur_area:
                return False, aid, locked
        self.ss_locked.append((cur_area, locker_id))
        return None

    def ssunlock(self, locker_id, ci, cj):
        cur_area = make_ss_area(ci, cj)
        self.ss_locked.remove((cur_area, locker_id))

    def _mod(self, i, j, ch):
        self.grid[i][j] += ch
        return self.grid[i][j]
   
    def get(self, i, j):
        return self.grid[i][j]

    def incr(self, i, j):
        return self._mod(i, j, 1)

    def decr(self, i, j):
        return self._mod(i, j, -1)

    def count_cigs(self, ci, cj, si, sj):
        return sum(sum(self.grid[i][j] for j in range(cj, cj + sj)) for i in range(ci, ci + si))

def read_int_line(fp, unpack=True):
    line = fp.readline()
    if line == '':
        return 0
    l = [int(x) for x in line.strip().split(' ')]
    if unpack and len(l) == 1:
        return l[0]
    return l

def read_pp(fp):
    aid, si, sj, gt, num_areas = read_int_line(fp)
    areas = [tuple(read_int_line(fp)) for _ in range(num_areas)]
    return PP(aid, si, sj, gt, areas)

class Order(Enum):
    BREAK = auto()
    STOP = auto()
    CONTINUE = auto()

def read_order(fp):
    time, order_type = fp.readline().strip().split(' ')
    time = int(time)
    if order_type == 'break':
        order = Order.BREAK
    elif order_type == 'continue':
        order = Order.CONTINUE
    elif order_type == 'stop':
        order = Order.STOP
    else:
        raise ValueError(f'Unknown order type: "{order_type}"')
    return time, order

def _ss_pack(lst):
    assert len(lst) == 3
    return (lst[0], lst[1]), lst[2]

def read_ss(fp):
    aid, ft, num_areas = read_int_line(fp)
    area_counts = [_ss_pack(read_int_line(fp)) for _ in range(num_areas)]
    return SS(aid, ft, area_counts)

def read_grid(fp):
    nrows, ncols = read_int_line(fp)
    return [read_int_line(fp, unpack=False) for _ in range(nrows)]

def read_initial_state(fp, odt, ort, gdt, ldt):
    grid = read_grid(fp)
    npps = read_int_line(fp)
    pps = [read_pp(fp) for _ in range(npps)]
    norders = read_int_line(fp)
    orders = [read_order(fp) for _ in range(norders)]
    nsss = read_int_line(fp)
    sss = [read_ss(fp) for _ in range(nsss)]
    return State(grid, pps, orders, sss, odt, ort, gdt, ldt)


def parse_arguments():
    parser = ArgumentParser(description='a "basic" consistency checker for hw2')
    parser.add_argument('input_file')
    parser.add_argument('output_file')
    parser.add_argument('--order-delay-tolerance', '-odt', type=int, default=10,
                        help='allowed order giving delay before penalty in ms')
    parser.add_argument('--order-reaction-tolerance', '-ort', type=int, default=10,
                        help='allowed order reaction delay before penalty in ms')
    parser.add_argument('--gather-delay-tolerance', '-gdt', type=int, default=10,
                        help='allowed delay from the expected gathering/flicking time in ms')
    parser.add_argument('--suppress-oks', '-s', action='store_true',
                        help='if specified, suppress OK messages, show only warnings '
                             'and errors.')
    parser.add_argument('--lock-delay-tolerance', '-ldt', type=int, default=0,
                        help='if specified as > 0, make sure PP\'s and SS\'s are locking '
                             'their target areas with this much delay at most, and not '
                             'blocking unnecessarily. Used in part 3 tests as a tougher '
                             'constraint as explained in the first case of the '
                             'complex_scenarios.pdf file.')
    return parser.parse_args()

def main(args):
    with open(args.input_file, 'r') as fp:
        state = read_initial_state(fp, args.order_delay_tolerance, args.order_reaction_tolerance,
                                   args.gather_delay_tolerance, args.lock_delay_tolerance)

    for notif in parse_notifications_from(args.output_file):
        state.apply(notif)

    state.exit_check()

    print('-' * 80)
    print('Looks like no fatal inconsistencies occurred! Good job.')
    print()

    perc = (100 - g_tally_penalties()) / 100
    print('Consistency multiplier:')
    print(f'{perc:.2f}')

    exit(0)

if __name__ == '__main__':
    args = parse_arguments()

    if args.suppress_oks:
        def approve(*args, **kwargs):
            pass
    else:
        def approve(*args, **kwargs):
            print('+ OK:', *args, **kwargs)
    
    main(args)
