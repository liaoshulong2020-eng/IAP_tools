import re

# Read the file
with open('can_app.c', 'r', encoding='utf-8') as f:
    content = f.read()

# Fix 1: Replace llc.custom_llc.custom_can_data with llc.custom_can_data
content = re.sub(r'llc\.custom_llc\.custom_can_data', 'llc.custom_can_data', content)

# Fix 2: Replace hld_llc.custom_can_data with llc.hld_can_data
content = re.sub(r'hld_llc\.custom_can_data', 'llc.hld_can_data', content)

# Fix 3: Replace hld_can_data with llc.hld_can_data (but not already fixed ones)
content = re.sub(r'hld_can_data\.', 'llc.hld_can_data.', content)

# Write back
with open('can_app.c', 'w', encoding='utf-8') as f:
    f.write(content)

print("Fixed can_app.c")
