from MainWindow import *
from HexFile import *

"""
打包命令：
C:\software\python\venv1\Scripts\pyinstaller -F -n IapFileCreator main.py
"""
################################################################################
# 主函数
################################################################################
def main():
    app=QApplication(sys.argv)
    
    #设置全局字体大小为12
    font=QApplication.font()
    font.setPointSize(10)
    QApplication.setFont(font)
    
    ui=MainWindow()
    ui.show()
    sys.exit(app.exec())
    
if __name__ == '__main__':
    main()
    #test
    # hex=HexFile()
    # hex.load("test/205B-LLC-偏移地址超过64K.hex")
