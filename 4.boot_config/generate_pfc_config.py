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

def read_hex_file(file_path):
    """读取Intel HEX文件并返回字节数组"""
    try:
        data = bytearray()
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                if line.startswith(':'):
                    # 解析Intel HEX格式
                    byte_count = int(line[1:3], 16)
                    record_type = int(line[7:9], 16)
                    
                    if record_type == 0:  # 数据记录
                        hex_data = line[9:9+byte_count*2]
                        for i in range(0, len(hex_data), 2):
                            data.append(int(hex_data[i:i+2], 16))
        return bytes(data)
    except FileNotFoundError:
        print(f"错误: 找不到文件 {file_path}")
        return None
    except Exception as e:
        print(f"错误: 读取HEX文件失败 - {e}")
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

def generate_pfc_bin_config():
    """生成PFC BIN配置文件"""
    bin_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.bin"
    
    # 检查文件是否存在
    if not os.path.exists(bin_path):
        bin_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.bin"
    
    if not os.path.exists(bin_path):
        print(f"错误: 找不到PFC bootloader二进制文件")
        return False
    
    # 读取二进制文件
    data = read_binary_file(bin_path)
    if data is None:
        return False
    
    # 生成C#数组
    file_name = os.path.basename(bin_path)
    comment = f"{file_name} ({len(data)} bytes)"
    csharp_code = bytes_to_csharp_array(data, "BOOTLOADER_5800_PFC_BIN_DATA", comment)
    
    # 写入文件
    output_path = "BOOTLOADER_5800_PFC_BIN_DATA.cs"
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(csharp_code)
        print(f"成功生成: {output_path}")
        print(f"文件大小: {len(data)} 字节")
        return True
    except Exception as e:
        print(f"错误: 写入文件失败 - {e}")
        return False

def generate_pfc_hex_config():
    """生成PFC HEX配置文件"""
    hex_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.hex"
    
    # 检查文件是否存在
    if not os.path.exists(hex_path):
        hex_path = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.hex"
    
    if not os.path.exists(hex_path):
        print(f"错误: 找不到PFC bootloader HEX文件")
        return False
    
    # 读取HEX文件内容（作为文本）
    try:
        with open(hex_path, 'r') as f:
            hex_content = f.read()
        
        # 转换为字节数组
        hex_bytes = hex_content.encode('utf-8')
        
        # 生成C#数组
        file_name = os.path.basename(hex_path)
        comment = f"{file_name} ({len(hex_bytes)} bytes)"
        csharp_code = bytes_to_csharp_array(hex_bytes, "BOOTLOADER_5800_PFC_HEX_DATA", comment)
        
        # 写入文件
        output_path = "BOOTLOADER_5800_PFC_HEX_DATA.cs"
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(csharp_code)
        print(f"成功生成: {output_path}")
        print(f"文件大小: {len(hex_bytes)} 字节")
        return True
    except Exception as e:
        print(f"错误: 处理HEX文件失败 - {e}")
        return False

def main():
    """主函数"""
    print("PFC Bootloader配置文件生成工具")
    print("=" * 50)
    
    # 切换到脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    success_count = 0
    
    # 生成BIN配置
    print("\n1. 生成PFC BIN配置文件...")
    if generate_pfc_bin_config():
        success_count += 1
    
    # 生成HEX配置
    print("\n2. 生成PFC HEX配置文件...")
    if generate_pfc_hex_config():
        success_count += 1
    
    print("\n" + "=" * 50)
    print(f"完成! 成功生成 {success_count}/2 个配置文件")
    
    if success_count == 2:
        print("\n✅ PFC配置支持已成功添加到boot_config目录")
        print("   - BOOTLOADER_5800_PFC_BIN_DATA.cs")
        print("   - BOOTLOADER_5800_PFC_HEX_DATA.cs")
        print("\n📝 下一步:")
        print("   1. 将这些文件集成到IAP工具项目中")
        print("   2. 更新TargetDetector.cs以支持PFC设备检测")
        print("   3. 更新MainForm.cs以支持PFC bootloader合并")
    else:
        print("\n❌ 部分文件生成失败，请检查错误信息")

if __name__ == "__main__":
    main()
