import argparse
import re
import sys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("logfile")
    ap.add_argument("--sent", default="SENT")
    ap.add_argument("--delivered", default="RECEIVED")
    ap.add_argument("--failed", default="FAILED")
    a = ap.parse_args()

    names = {"sent": a.sent, "delivered": a.delivered, "failed": a.failed}
    ids = {k: set() for k in names}

    ev_re = re.compile(r'\[(?:E|S|N|EMERGENCY|SYSTEM|NORMAL)\]\s*\[([A-Z_]+)\]|"ev"\s*:\s*"([A-Z_]+)"')
    id_re = re.compile(r'id=(\d+)|"id"\s*:\s*(\d+)')

    with open(a.logfile, encoding="utf-8", errors="replace") as f:
        for line in f:
            em, im = ev_re.search(line), id_re.search(line)
            if not em or not im:
                continue
            ev = (em.group(1) or em.group(2)).replace("EV_", "")
            mid = int(im.group(1) or im.group(2))
            for key, name in names.items():
                if ev == name:
                    ids[key].add(mid)

    sent, deliv, failed = (len(ids[k]) for k in ("sent", "delivered", "failed"))
    lost = ids["sent"] - ids["delivered"] - ids["failed"]

    print(f"sent       : {sent}")
    print(f"delivered  : {deliv}")
    print(f"FAILED     : {failed}")
    print(f"unaccounted: {len(lost)}  (sent but neither delivered nor FAILED)")
    if sent:
        print(f"delivered rate: {100.0 * deliv / sent:.2f}%   (target 99%+ within 3 retries)")
    if lost:
        print("ids still unaccounted:", sorted(lost)[:20], "..." if len(lost) > 20 else "")
        print("-> either still in flight when the run ended, or silently lost (check!)")
        sys.exit(1)


if __name__ == "__main__":
    main()
