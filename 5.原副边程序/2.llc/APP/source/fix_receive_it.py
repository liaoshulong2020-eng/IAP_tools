import re

with open('can_app.c', 'r', encoding='utf-8') as f:
    content = f.read()

# Fix the LL_CAN_Receive_IT call to add (uint32_t*) cast
pattern = r'LL_CAN_Receive_IT\(user_can_ctrl\.Instance,\s+&user_can_ctrl\.rxbuf_fmt\[user_can_ctrl\.rx_cnt\],\s+user_can_ctrl\.rx_buf\[user_can_ctrl\.rx_cnt\]\);'
replacement = 'LL_CAN_Receive_IT(user_can_ctrl.Instance,\n                         &user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt],\n                         (uint32_t*)user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt]);'

content = re.sub(pattern, replacement, content)

with open('can_app.c', 'w', encoding='utf-8') as f:
    f.write(content)

print("Fixed LL_CAN_Receive_IT")
