import re

# Read the file
with open('can_app.c', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Process line by line
for i, line in enumerate(lines):
    # Fix line 394: can_send_data((void*)&can_data.power_para -> can_send_data((void*)&llc.custom_can_data.power_para
    if 'can_send_data((void*)&llc.custom_can_data.power_para' in line:
        lines[i] = line.replace('can_send_data((void*)&llc.custom_can_data.power_para', 
                                'can_send_data((void*)&llc.custom_can_data.power_para')
    
    # Fix line 402: hld_can_data.version_info -> llc.hld_can_data.version_info
    if 'llc.llc.hld_can_data' in line:
        lines[i] = line.replace('llc.llc.hld_can_data', 'llc.hld_can_data')
    
    # Fix line 404: can_data.version_info -> llc.custom_can_data.version_info
    if 'can_send_data((void*)&llc.custom_can_data.version_info' in line:
        lines[i] = line.replace('can_send_data((void*)&llc.custom_can_data.version_info',
                                'can_send_data((void*)&llc.custom_can_data.version_info')

# Write back
with open('can_app.c', 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("Fixed can_app.c v2")
