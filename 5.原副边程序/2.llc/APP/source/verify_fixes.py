import re

with open('can_app.c', 'r', encoding='utf-8') as f:
    content = f.read()
    lines = content.split('\n')

issues = []

# Check 1: llc.can_addr should be llc.custom_can_data.can_addr
for i, line in enumerate(lines, 1):
    if 'llc.can_addr' in line and 'llc.custom_can_data.can_addr' not in line:
        issues.append(f"Line {i}: Found llc.can_addr (should be llc.custom_can_data.can_addr)")

# Check 2: Missing #include <string.h>
if '#include <string.h>' not in content:
    issues.append("Missing: #include <string.h>")

# Check 3: Missing crc8 declaration
if 'uint8_t crc8(uint8_t *data, uint32_t len);' not in content:
    issues.append("Missing: uint8_t crc8(uint8_t *data, uint32_t len);")

# Check 4: DeviceInfo devices[6] should be removed
if 'DeviceInfo devices[6]' in content:
    issues.append("Error: DeviceInfo devices[6] should be removed")

# Check 5: Check for can_data. references (should be llc.custom_can_data.)
for i, line in enumerate(lines, 1):
    if 'can_data.' in line and 'llc.custom_can_data.' not in line and 'hld_can_data' not in line:
        issues.append(f"Line {i}: Found can_data. (should be llc.custom_can_data.)")

# Check 6: Check for hld_can_data. references (should be llc.hld_can_data.)
for i, line in enumerate(lines, 1):
    if 'hld_can_data.' in line and 'llc.hld_can_data.' not in line:
        issues.append(f"Line {i}: Found hld_can_data. (should be llc.hld_can_data.)")

# Check 7: Check for user_data.shareloop_kp (should be llc.shareloop_kp_init)
for i, line in enumerate(lines, 1):
    if 'user_data.shareloop_kp' in line:
        issues.append(f"Line {i}: Found user_data.shareloop_kp (should be llc.shareloop_kp_init)")

# Check 8: Check for user_data.shareloop_ki (should be llc.shareloop_ki_init)
for i, line in enumerate(lines, 1):
    if 'user_data.shareloop_ki' in line:
        issues.append(f"Line {i}: Found user_data.shareloop_ki (should be llc.shareloop_ki_init)")

# Check 9: Check for LL_CAN_Receive_IT parameter type
for i, line in enumerate(lines, 1):
    if 'LL_CAN_Receive_IT' in line and 'user_can_ctrl.rx_buf' in line:
        if '(uint32_t*)' not in line:
            issues.append(f"Line {i}: LL_CAN_Receive_IT missing (uint32_t*) cast")

if issues:
    print("Issues found:")
    for issue in issues:
        print(f"  {issue}")
else:
    print("All fixes verified successfully!")
