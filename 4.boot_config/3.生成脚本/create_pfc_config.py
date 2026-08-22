#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PFC Bootloader配置文件生成工具
用于将PFC bootloader的bin和hex文件转换为C#数组格式
"""

import os
import sys

def read_binary_file(file_path):
    """读取二进制文件并返回字节数组"""
    try:
        with open(file_path, 'rb') as f:
            return f.read()
    except FileNotFoundError:
        print(f"错误: 找不到文件 {file_path}")
        return None
    except Exception as e:
        print(f"错误: 读取文件失败 - {e}")
        return None

def bytes_to_csharp_array(data, array_name, comment=""):
    """将字节数组转换为C#数组格式"""
    if not data:
        return ""
    
    lines = []
    if comment:
        lines.append(f"// {comment}")
    
    lines.append(f"private static readonly byte[] {array_name} = new byte[]")
    lines.append("{")
    
    # 每行16个字节
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_values = [f"0x{b:02X}" for b in chunk]
        line = "    " + ", ".join(hex_values)
        if i + 16 < len(data):
            line += ","
        lines.append(line)
    
    lines.append("};")
    return "\n".join(lines)

def main():
    """主函数"""
    print("PFC Bootloader配置文件生成工具")
    print("=" * 50)
    
    # 定义文件路径
    bin_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.bin"
    hex_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.hex"
    
    # 如果patched文件不存在，使用原始文件
    if not os.path.exists(bin_path):
        bin_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.bin"
    
    if not os.path.exists(hex_path):
        hex_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.hex"
    
    success_count = 0
    
    # 生成BIN配置
    print("\n1. 生成PFC BIN配置文件...")
    if os.path.exists(bin_path):
        data = read_binary_file(bin_path)
        if data is not None:
            file_name = os.path.basename(bin_path)
            comment = f"{file_name} ({len(data)} bytes)"
            csharp_code = bytes_to_csharp_array(data, "BOOTLOADER_5800_PFC_BIN_DATA", comment)
            
            output_path = "BOOTLOADER_5800_PFC_BIN_DATA.cs"
            try:
                with open(output_path, 'w', encoding='utf-8') as f:
                    f.write(csharp_code)
                print(f"✅ 成功生成: {output_path}")
                print(f"   文件大小: {len(data)} 字节")
                success_count += 1
            except Exception as e:
                print(f"❌ 写入文件失败 - {e}")
    else:
        print(f"❌ 找不到文件: {bin_path}")
    
    # 生成HEX配置
    print("\n2. 生成PFC HEX配置文件...")
    if os.path.exists(hex_path):
        try:
            with open(hex_path, 'r') as f:
                hex_content = f.read()
            
            # 转换为字节数组
            hex_bytes = hex_content.encode('utf-8')
            
            file_name = os.path.basename(hex_path)
            comment = f"{file_name} ({len(hex_bytes)} bytes)"
            csharp_code = bytes_to_csharp_array(hex_bytes, "BOOTLOADER_5800_PFC_HEX_DATA", comment)
            
            output_path = "BOOTLOADER_5800_PFC_HEX_DATA.cs"
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(csharp_code)
            print(f"✅ 成功生成: {output_path}")
            print(f"   文件大小: {len(hex_bytes)} 字节")
            success_count += 1
        except Exception as e:
            print(f"❌ 处理HEX文件失败 - {e}")
    else:
        print(f"❌ 找不到文件: {hex_path}")
    
    print("\n" + "=" * 50)
    print(f"完成! 成功生成 {success_count}/2 个配置文件")
    
    if success_count == 2:
        print("\n✅ PFC配置支持已成功添加到boot_config目录")
        print("   - BOOTLOADER_5800_PFC_BIN_DATA.cs")
        print("   - BOOTLOADER_5800_PFC_HEX_DATA.cs")
    else:
        print("\n❌ 部分文件生成失败，请检查错误信息")

if __name__ == "__main__":
    main()
