import sys
import time
import os
import json
from PyQt6.QtWidgets import *
from PyQt6.QtCore import *
from crc import *
from HexFile import *

################################################################################
# 主窗口
################################################################################
class MainWindow(QMainWindow):
	bootFile=""
	appFile=""
	outFile=""
	bootMaxSize=0
	argBaseAddr=0
	appBaseAddr=0
	appMaxSize=0
	flashBaseAddr=0
	_flashBaseAddrFlag=False
	
	bootSize=0
	bootData=None
	appSize=0
	appData=None
	#构造函数
	def __init__(self,parent=None):
		super(MainWindow,self).__init__(parent)
		self.setFixedSize(500,380)
		self.setWindowTitle("IapFileCreator")
	
		############################################################
		# UI排版
		############################################################
	
		#加载项目
		self.btnLoad=QPushButton(self)
		self.btnLoad.setGeometry(100,20,100,30)
		self.btnLoad.setText("加载项目")
		self.btnLoad.clicked.connect(self.slotBtnLoad)
		
		#生成输出文件
		self.btnCreate=QPushButton(self)
		self.btnCreate.setGeometry(300,20,100,30)
		self.btnCreate.setText("生成输出文件")
		self.btnCreate.clicked.connect(self.slotBtnCreate)
		
		#信息框
		self.textEdit=QTextEdit(self)
		self.textEdit.setGeometry(20,70,460,290)
		self.textEdit.setReadOnly(True)
		self.textEdit.document().setMaximumBlockCount(1000)  #设置最大行数
		
	#textEdit显示常规文本
	def showNormalText(self,text):
		ftext="<p style=\"color:black;font-size:12pt;margin:0\">%s</p>"%(text)
		self.textEdit.insertHtml(ftext)
		self.textEdit.insertPlainText("\n")
		
	#textEdit显示OK文本
	def showOkText(self,text):
		ftext="<p style=\"color:#0000FF;font-size:12pt;margin:0\">%s</p>"%(text)
		self.textEdit.insertHtml(ftext)
		self.textEdit.insertPlainText("\n")
		
	#textEdit显示NG文本
	def showNgText(self,text):
		ftext="<p style=\"color:#FF0000;font-size:12pt;margin:0\">%s</p>"%(text)
		self.textEdit.insertHtml(ftext)
		self.textEdit.insertPlainText("\n")

	#加载json文件
	def _loadJsonFile(self,path):
		fp=open(path)
		fdata=fp.read()
		fp.close()
		self.textEdit.clear()
		jsonDict=json.loads(fdata)
		#检查必要字段
		keys=["bootFile","appFile","outFile","bootMaxSize","argBaseAddr","appBaseAddr","appMaxSize"]
		for key in keys:
			if key not in jsonDict:return False
			
		self.bootFile=jsonDict["bootFile"]
		self.appFile=jsonDict["appFile"]
		self.outFile=jsonDict["outFile"]
		self.bootMaxSize=jsonDict["bootMaxSize"]
		self.argBaseAddr=jsonDict["argBaseAddr"]
		self.appBaseAddr=jsonDict["appBaseAddr"]
		self.appMaxSize=jsonDict["appMaxSize"]
		if self._isHexFileName(self.outFile):
			if "flashBaseAddr" not in jsonDict:return False
			else:
				self.flashBaseAddr=jsonDict["flashBaseAddr"]
				self._flashBaseAddrFlag=True
				
		#显示项目信息
		self.showNormalText("bootFile: %s"%(self.bootFile))
		self.showNormalText("aappFile: %s"%(self.appFile))
		self.showNormalText("outFile: %s"%(self.outFile))
		kb=int(self.bootMaxSize/1024)
		self.showNormalText("maxBootSize=%d KB"%(kb))
		self.showNormalText("argBaseAddr=0x%08X"%(self.argBaseAddr))
		self.showNormalText("appBaseAddr=0x%08X"%(self.appBaseAddr))
		
		return True
		
	#判断是否Hex文件名
	def _isHexFileName(self,fileName):
		ext=fileName[-4:].lower()
		# print("extName=",ext)
		if ext==".hex":return True
		return False
		
	#加载bootloader文件
	def loadBootFile(self):
		#如果是Hex文件
		if self._isHexFileName(self.bootFile):
			hex=HexFile()
			self.bootData=hex.load(self.bootFile)
			if self.bootData is None:
				# print("error: boot文件错误")
				self.showNgText("error：加载boot文件失败")
				return False
			self.bootSize=hex.getFileSize()
		#如果是bin文件
		else:
			bootSize=os.path.getsize(self.bootFile)
			# print("boot文件大小:",bootSize)
			if bootSize<=0 or bootSize>self.bootMaxSize:
				# print("error: boot文件错误")
				self.showNgText("error：加载boot文件失败")
				return False
			self.bootSize=bootSize
			file=open(self.bootFile,"rb")
			#加载文件数据
			self.bootData=file.read()
			file.close()
		sv="OK：加载boot文件，%d bytes"%(self.bootSize)
		self.showOkText(sv)
		return True
		
	#加载APP程序文件
	def loadAppFile(self):
		#如果是Hex文件
		if self._isHexFileName(self.appFile):
			hex=HexFile()
			self.appData=hex.load(self.appFile)
			if self.appData is None:
				# print("error: APP程序文件错误")
				self.showNgText("error：加载APP程序文件失败")
				return False
			self.appSize=hex.getFileSize()
		#如果是bin文件
		else:
			app_size=os.path.getsize(self.appFile)
			# print("APP文件大小:",app_size)
			if app_size<=0 or app_size>self.appMaxSize:
				# print("error: APP程序文件错误")
				self.showNgText("error：加载APP程序文件失败")
				return False
			self.appSize=app_size
			file=open(self.appFile,"rb")
			#加载文件数据
			self.appData=file.read()
			file.close()
		#计算crc校验码
		self.appCrc=crc32_update(0,self.appData,self.appSize)
		print("APP文件CRC32=0x%08X"%(self.appCrc))
		sv="OK：加载APP文件，%d bytes"%(self.appSize)
		self.showOkText(sv)
		return True
		
	#槽函数：加载配置文件
	def slotBtnLoad(self):
		#选择单个文件
		path,ext=QFileDialog.getOpenFileName(self,'打开文件','','json文件 (*.json)')
		if path is None or len(path)<=0: return
		# print("path=",path)
		if self._loadJsonFile(path)==False:self.showNgText("error: 配置文件语法错误")
		
	#槽函数：生成输出文件
	def slotBtnCreate(self):
		if len(self.bootFile)<=0:
			self.showNgText("请先加载项目")
			return
		self.showNormalText("")
		if self.loadBootFile()==False:return
		if self.loadAppFile()==False:return
		#创建缓冲区
		fileSize=self.bootMaxSize+self.appSize
		fileData=bytearray(fileSize)
		#复制boot文件
		index=0
		while index<self.bootSize:
			fileData[index]=self.bootData[index]
			index+=1
		#复制APP文件大小
		fileData[self.argBaseAddr+0]=self.appSize&0xff
		fileData[self.argBaseAddr+1]=(self.appSize>>8)&0xff
		fileData[self.argBaseAddr+2]=(self.appSize>>16)&0xff
		fileData[self.argBaseAddr+3]=(self.appSize>>24)&0xff
		#复制CRC
		fileData[self.argBaseAddr+4]=self.appCrc&0xff
		fileData[self.argBaseAddr+5]=(self.appCrc>>8)&0xff
		fileData[self.argBaseAddr+6]=(self.appCrc>>16)&0xff
		fileData[self.argBaseAddr+7]=(self.appCrc>>24)&0xff
		# print("crc1=0x%02X"%(fileData[self.argBaseAddr]))
		# print("crc2=0x%02X"%(fileData[self.argBaseAddr+1]))
		# print("crc3=0x%02X"%(fileData[self.argBaseAddr+2]))
		# print("crc4=0x%02X"%(fileData[self.argBaseAddr+3]))
		#复制APP文件
		index=0
		while index<self.appSize:
			fileData[self.appBaseAddr+index]=self.appData[index]
			index+=1
			
		#创建输出文件
		#如果是Hex文件
		if self._isHexFileName(self.outFile):
			hex=HexFile()
			hex.outHexFile(self.flashBaseAddr,fileData,fileSize,self.outFile)
		#如果是bin文件
		else:
			file=open(self.outFile,"wb")
			file.write(fileData)
			file.close()
		self.showOkText("OK: 创建MCU程序文件成功")