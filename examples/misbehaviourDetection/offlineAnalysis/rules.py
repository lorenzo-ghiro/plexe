import numpy as np
from shapely.geometry import Point, Polygon
import code  # code.interact(local=dict(globals(), **locals()))
from math import cos, sin, radians, hypot, atan2, degrees


# JERK TOLERANCE RANGES FOR TIME-SCALE 1s
TS = 1  # Time-Scale [s]

JERK_RSX = 0          # [m/s/s/s]
JERK_FSX = 0          # [m/s/s/s]
JERK_FDX = 8          # [m/s/s/s]
JERK_RDX = 20         # [m/s/s/s]

ALPHAs = 0.1            # 10% tolerance for flat zone for speed
BETAs = 0.25            # 25% tolerance for triangular zones for speed
ALPHAp = 0.2            # 20% tolerance for flat zone for position
BETAp = 0.3             # 30% tolerance for triangular zones for position

SMOOTHING_FACTOR = 0.9
MAX_TX_RADIUS = 220   # [m] as TXRange estimation for Urban Scenario
GPS_UNCERTAINITY = 1  # [m]

MAX_DELTA_T = 1.3    # [s]
MAX_DELTA_HED = 160  # degrees

MINPOSTOL = 0.5
MINPOSRANGE = 1

MINSPEEDTOL = 1    # [m/s]
MINSPEEDRANGE = 2  # [m/s]

NAN = float('nan')


class Message:
    def __init__(self, sendTime, posx, posy, spdx, spdy, acl, hed, label):
        # Time the message was sent (e.g., in seconds)
        self.sendTime = sendTime
        self.posx = posx          # X position
        self.posy = posy          # Y position
        self.spdx = spdx          # Speed in X direction
        self.spdy = spdy          # Speed in Y direction
        self.acl = acl            # Acceleration
        self.hed = hed            # Heading
        self.label = label        # Label

    def __repr__(self):
        return (f"Message(sendTime={self.sendTime:.2f}, x={self.posx:.2f}, y={self.posy:.2f}, "
                f"spdx={self.spdx:.2f}, spdy={self.spdy:.2f}, acl={self.acl:.2f}, "
                f"hed={self.hed:.2f}, label={self.label})")


def plainsum(vals):
    return sum(vals)


def satur_sum(vals):
    return min([1.0, plainsum(vals)])


def plainmax(vals):
    return max(vals)


def norm_sum(vals):
    return plainsum(vals) / len(vals)


def memory_score(vals, mem_score):
    return SMOOTHING_FACTOR * mem_score + norm_sum(vals)


def satur_memscore(vals, mem_score):
    return min(1.0, memory_score(vals, mem_score))


def smooth_risk(vals, low_thresh=0.3, high_thresh=0.7):
    vals = np.array(list(vals))

    # component 1: high risk if at least one is high
    # it already tends to 1 if one is high
    risk_if_one_high = np.max(vals)

    # component 2: high risk if no one is low
    # it grows if the minimum grows
    no_low_penalty = 1.0 - np.exp(-np.min(vals) / low_thresh)

    # component 3: low risk only if all are low
    # tends to 0 if the average is low
    all_low_boost = 1.0 - np.exp(-np.mean(vals) / low_thresh)

    # weighted combination (all between 0 and 1)
    return np.clip((0.5 * risk_if_one_high + 0.3 * no_low_penalty + 0.2 * all_low_boost), 0.0, 1.0)


def evaluate_message_pair(msg_prev, msg, mem_score, rxvpos, ARTenabled):
    if np.isnan(mem_score):
        mem_score = 0
    deltaT = msg.sendTime - msg_prev.sendTime
    deltaHed = angular_distance(msg_prev.hed, msg.hed)
    assert deltaT > 0
    if deltaT > MAX_DELTA_T or deltaHed > MAX_DELTA_HED:
        if (ARTenabled):
            return {'norm_sum': NAN, 'memory_score': NAN}, NAN, {'js': NAN, 'ss': NAN, 'ps': NAN, 'arts': NAN}, {'jerk': NAN, 'se': NAN, 'pe': NAN}
        else:
            return {'norm_sum': NAN, 'memory_score': NAN}, NAN, {'js': NAN, 'ss': NAN, 'ps': NAN}, {'jerk': NAN, 'se': NAN, 'pe': NAN}
    js, jerk = computeJerkScore(msg_prev, msg, deltaT)
    ss, se, avgspeed_est_modulo = computeSpeedScore(msg_prev, msg, deltaT)
    ps, pe = computePosScore(msg_prev, msg, deltaT, avgspeed_est_modulo)

    scores = {'js': js, 'ss': ss, 'ps': ps}
    if (ARTenabled):
        arts = artScore(msg_prev, msg, deltaT, rxvpos)
        scores['arts'] = arts

    errors = {'jerk': jerk, 'se': se, 'pe': pe}

    score_vals = scores.values()

    new_mem_score = memory_score(score_vals, mem_score)
    metrics = {'norm_sum': norm_sum(score_vals), 'memory_score': new_mem_score}

    return metrics, new_mem_score, scores, errors


def computePlausibility(value, rsx, rdx, fsx, fdx):
    assert rsx <= fsx and fsx <= fdx and fdx < rdx

    if rsx <= value < fsx:
        return (value - rsx) / (fsx - rsx)
    elif fsx <= value < fdx:
        return 1
    elif fdx <= value < rdx:
        return (rdx - value) / (rdx - fdx)
    else:
        return 0


def catchRangePlausibilityScore(p1, p2, rt=MAX_TX_RADIUS, rgps=GPS_UNCERTAINITY):
    """
    Calculate the fraction of the areas of the circles centered at p1 and p2
    (radius rgps) that is contained within the transmission circle
    centered at the midpoint (radius rt).

    Parameters:
    - p1, p2: tuples (x, y), Cartesian coordinates
    - rt: transmission circle radius
    - rgps: radius of the GPS uncertainty circles (centered at p1 and p2)

    Returns:
    - dictionary with midpoint, areas and coverage fraction
    """

    # Circles centered at p1 and p2
    c1 = Point(p1).buffer(rgps)
    c2 = Point(p2).buffer(rgps)

    # Midpoint
    pm = ((p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2)

    # Transmission circle centered at pm
    ct = Point(pm).buffer(rt)

    # Areas of the circles
    area_c1 = c1.area
    area_c2 = c2.area

    # Intersections with the transmission circle
    inter_c1 = ct.intersection(c1).area
    inter_c2 = ct.intersection(c2).area

    # Fraction of overlapping area
    tot_area = area_c1 + area_c2
    overlapping_area = inter_c1 + inter_c2
    fraction = overlapping_area / tot_area if tot_area > 0 else 0
    return np.clip(fraction, 0.0, 1.0)


def artScore(msg_prev, msg, deltaT, rxvpos):
    plau = catchRangePlausibilityScore(p1=(msg.posx, msg.posy), p2=rxvpos)
    return 1 - plau


def computeJerkScore(msg_prev, msg, deltaT):
    jerk = abs(msg.acl - msg_prev.acl) / deltaT  # m/s / s --> m/s^3
    # if small deltaT, allow jerk to be larger
    sf = TS / deltaT  # sf aka scaleFactor, the smaller DeltaT, the larger the sf
    plau = computePlausibility(
        jerk, JERK_RSX*sf, JERK_RDX*sf, JERK_FSX*sf, JERK_FDX*sf)
    return 1 - plau, jerk


def computeSpeedScore(msg_prev, msg, deltaT):
    accX_prev = msg_prev.acl * np.cos(np.radians(msg_prev.hed))
    accY_prev = msg_prev.acl * np.sin(np.radians(msg_prev.hed))

    speedX_est = msg_prev.spdx + accX_prev * deltaT
    speedY_est = msg_prev.spdy + accY_prev * deltaT

    speed_err = hypot(speedX_est - msg.spdx, speedY_est - msg.spdy)

    # accumualte Error Factor, the larger DeltaT, the larger the tolerated cumulated error
    AEF = (deltaT/TS)
    avgspeed_est_modulo = hypot(
        (msg_prev.spdx + speedX_est)/2, (msg_prev.spdy + speedY_est)/2)
    flat_tolerance = ALPHAs * AEF * avgspeed_est_modulo  # m/s
    # at least MINSPEEDTOL tolerance always
    flat_tolerance = max(flat_tolerance, MINSPEEDTOL)

    range_tolerance = BETAs * AEF * avgspeed_est_modulo
    range_tolerance = max(range_tolerance, MINSPEEDRANGE)

    plau = computePlausibility(speed_err, rsx=0, rdx=range_tolerance,
                               fsx=0, fdx=flat_tolerance)
    return 1 - plau, speed_err, avgspeed_est_modulo


def computePosScore(msg_prev, msg, deltaT, avgspeed_est_modulo):
    accX_prev = msg_prev.acl * np.cos(np.radians(msg_prev.hed))
    accY_prev = msg_prev.acl * np.sin(np.radians(msg_prev.hed))

    posX_est = msg_prev.posx + msg_prev.spdx * \
        deltaT + 0.5 * accX_prev * (deltaT ** 2)
    posY_est = msg_prev.posy + msg_prev.spdy * \
        deltaT + 0.5 * accY_prev * (deltaT ** 2)

    pos_err = hypot(posX_est - msg.posx, posY_est - msg.posy)

    # accumualte Error Factor, the larger DeltaT, the larger the tolerated cumulated error
    AEF = (deltaT/TS)
    avgmove_est = avgspeed_est_modulo * deltaT
    flat_tolerance = ALPHAp * AEF * avgmove_est  # m/s * s = m
    # at least MINPOSTOL tolerance always
    flat_tolerance = max(flat_tolerance, MINPOSTOL)

    range_tolerance = BETAp * AEF * avgmove_est  # m/s * s = m
    range_tolerance = max(range_tolerance, MINPOSRANGE)

    plau = computePlausibility(pos_err, rsx=0, rdx=range_tolerance,
                               fsx=0, fdx=flat_tolerance)
    return 1 - plau, pos_err


def angular_distance(start_deg, end_deg):
    start_deg = start_deg % 360
    end_deg = end_deg % 360

    # Calculate the shortest angular distance (β), in the range [0, 180]
    beta = (end_deg - start_deg + 540) % 360 - 180
    return abs(beta)
