#!/usr/bin/env python3
"""Static regression checks for the single printf-based embedded web GUI.
Does not replace building the ESP32 firmware in PlatformIO.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parent.parent
src = (root / 'src/main.cpp').read_text(encoding='utf-8')
config = (root / 'src/config.h').read_text(encoding='utf-8')
assert '#define FW_VERSION "1.5.4"' in config
start = src.index('    snprintf(buf, BUFSZ,')
end = src.index('    g_web.send(200, "text/html", buf);', start)
fragment = src[start:end]
# Between the first format literal and the first true printf argument.
argument_start = fragment.index('        g_settings.homeLat, g_settings.homeLon, gpsRow.c_str()')
format_code = fragment[:argument_start]
fmt_literals = re.findall(r'"(?:\\.|[^"\\])*"', format_code)
text = ''.join(bytes(x[1:-1], 'utf-8').decode('unicode_escape') for x in fmt_literals)
fmt_list = re.findall(r'%(?:[-+ #0]*\d*(?:\.\d*)?[hljztL]*[a-zA-Z%])', text)
fmt_list = [x for x in fmt_list if x != '%%']
args_part = fragment[argument_start:]
args_part = args_part[:args_part.rfind(');')]
# Split only on commas outside parentheses and quoted values.
args = []
buf, level, quote, escaped = '', 0, None, False
for ch in args_part:
    if escaped:
        escaped = False
    elif ch == '\\' and quote:
        escaped = True
    elif quote:
        if ch == quote: quote = None
    elif ch in "\"'": quote = ch
    elif ch == '(': level += 1
    elif ch == ')': level -= 1
    if ch == ',' and not quote and level == 0:
        args.append(buf.strip());buf=''
    else:
        buf += ch
if buf.strip(): args.append(buf.strip())
assert len(fmt_list) == len(args), (len(fmt_list), len(args), fmt_list, args)
for f, a in zip(fmt_list, args):
    if f.endswith('s'):
        assert '.c_str()' in a or '"' in a, (f, a)
    elif f.endswith('d'):
        assert '.c_str()' not in a, (f, a)
    elif f.endswith('f'):
        assert 'homeLat' in a or 'homeLon' in a, (f, a)
assert 'id=feedsource' in text
assert 'id=feedlast' in text
assert 'id=feedstate' in text
assert "/feedstatus" in text
assert "/altmax" in text
assert 'Maximum altitude' in text
assert 'setInterval(fs,10000)' in text
assert 'g_web.on("/altmax", handleAltMax)' in src
assert 'g_web.on("/feedstatus", HTTP_GET, handleFeedStatus)' in src
assert 'p.putInt("maxalt", g_maxAltFt)' in src
assert 'p.getInt("maxalt", 0)' in src
assert 'mbedtls_platform_set_calloc_free(tls_calloc, tls_free)' in src
print('PASS web printf placeholders, controls, endpoint, NVS and TLS hook')
