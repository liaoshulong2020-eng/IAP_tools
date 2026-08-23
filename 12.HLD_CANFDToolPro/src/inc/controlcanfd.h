#ifndef CONTROLCANFD_H
#define CONTROLCANFD_H

// ============================================================================
// ControlCANFD.h
// 珠海创芯科技 USBCANFD 系列设备的二次开发接口声明。
// 依据官方 ControlCANFD.h / zlgcan.h / config.h 整理，供 Qt 工程直接使用。
// ============================================================================

// Qt/MinGW 环境下的基础类型宏（官方头文件要求打开）
#ifndef CONTROL_CAN_TYPE_DEFINED
#define CONTROL_CAN_TYPE_DEFINED
typedef unsigned int        UINT;
typedef unsigned char       BYTE;
typedef unsigned short      USHORT;
typedef unsigned char       UCHAR;
typedef unsigned long long  UINT64;
typedef char                CHAR;
typedef unsigned int        DWORD;
typedef unsigned long       ULONG;
typedef void*               PVOID;
typedef int                 INT;
#endif

// ---------------------------------------------------------------------------
// 接口卡 / 设备类型定义
// ---------------------------------------------------------------------------
#define ZCAN_USBCANFD_200U          41      // USB-CAN FD 200U 双通道设备
#define ZCAN_USBCANFD_100U          42
#define ZCAN_USBCANFD_MINI          43

#define VCI_USBCAN_E_U              20
#define VCI_USBCAN_2E_U             21

// 函数调用返回状态值
#define STATUS_ERR                  0
#define STATUS_OK                   1
#define STATUS_ONLINE               2
#define STATUS_OFFLINE              3
#define STATUS_UNSUPPORTED          4

#define INVALID_DEVICE_HANDLE       0
#define INVALID_CHANNEL_HANDLE      0

// ---------------------------------------------------------------------------
// CAN 帧 ID 标志位
// ---------------------------------------------------------------------------
#define CAN_EFF_FLAG    0x80000000U     // EFF/SFF 标志（最高位）
#define CAN_RTR_FLAG    0x40000000U     // 远程帧标志
#define CAN_ERR_FLAG    0x20000000U     // 错误帧标志
#define CAN_ID_FLAG     0x1FFFFFFFU     // ID 掩码

#define CAN_SFF_MASK    0x000007FFU     // 标准帧 ID 范围
#define CAN_EFF_MASK    0x1FFFFFFFU     // 扩展帧 ID 范围
#define CAN_ERR_MASK    0x1FFFFFFFU

#define MAKE_CAN_ID(id, eff, rtr, err) \
    ((id) | (!!(eff) << 31) | (!!(rtr) << 30) | (!!(err) << 29))
#define IS_EFF(id)      (!!((id) & CAN_EFF_FLAG))   // 1:扩展帧 0:标准帧
#define IS_RTR(id)      (!!((id) & CAN_RTR_FLAG))   // 1:远程帧 0:数据帧
#define IS_ERR(id)      (!!((id) & CAN_ERR_FLAG))   // 1:错误帧 0:普通帧
#define GET_ID(id)      ((id) & CAN_ID_FLAG)

#define CAN_MAX_DLEN    8
#define CANFD_MAX_DLEN  64

#define TYPE_CAN        0
#define TYPE_CANFD      1

// CANFD BRS 标志（flags 字段 bit0）
#define CANFD_BRS       0x01

// ---------------------------------------------------------------------------
// CAN 帧结构体（ZCAN 接口）
// ---------------------------------------------------------------------------
typedef struct {
    UINT    can_id;     // 32 位 MAKE_CAN_ID + EFF/RTR/ERR 标志
    BYTE    can_dlc;    // 数据长度 (0 .. CAN_MAX_DLEN)
    BYTE    __pad;      // 对齐
    BYTE    __res0;     // 保留
    BYTE    __res1;     // 保留
    BYTE    data[CAN_MAX_DLEN];
} can_frame;

typedef struct {
    UINT    can_id;     // 32 位 MAKE_CAN_ID + EFF/RTR/ERR 标志
    BYTE    len;        // 数据长度 (0 .. CANFD_MAX_DLEN)
    BYTE    flags;      // CAN FD 附加标志（bit0: BRS）
    BYTE    __res0;     // 保留
    BYTE    __res1;     // 保留
    BYTE    data[CANFD_MAX_DLEN];
} canfd_frame;

// ---------------------------------------------------------------------------
// 设备信息
// ---------------------------------------------------------------------------
typedef struct tagZCAN_DEVICE_INFO {
    USHORT  hw_Version;         // 硬件版本号（十六进制）
    USHORT  fw_Version;         // 固件版本号
    USHORT  dr_Version;         // 驱动版本号
    USHORT  in_Version;         // 接口库版本号
    USHORT  irq_Num;            // 中断号
    BYTE    can_Num;            // 通道数
    UCHAR   str_Serial_Num[20]; // 序列号
    UCHAR   str_hw_Type[40];    // 硬件类型
    USHORT  reserved[4];
} ZCAN_DEVICE_INFO;

// ---------------------------------------------------------------------------
// 通道初始化配置
// ---------------------------------------------------------------------------
typedef struct tagZCAN_CHANNEL_INIT_CONFIG {
    UINT can_type;              // TYPE_CAN(0) / TYPE_CANFD(1)
    union {
        struct {
            UINT  acc_code;
            UINT  acc_mask;
            UINT  reserved;
            BYTE  filter;
            BYTE  timing0;
            BYTE  timing1;
            BYTE  mode;
        } can;
        struct {
            UINT   acc_code;
            UINT   acc_mask;
            UINT   abit_timing;
            UINT   dbit_timing;
            UINT   brp;
            BYTE   filter;
            BYTE   mode;        // 0:正常模式 1:只听模式
            USHORT pad;
            UINT   reserved;
        } canfd;
    };
} ZCAN_CHANNEL_INIT_CONFIG;

// ---------------------------------------------------------------------------
// 通道错误信息 / 状态
// ---------------------------------------------------------------------------
typedef struct tagZCAN_CHANNEL_ERR_INFO {
    UINT error_code;
    BYTE passive_ErrData[3];
    BYTE arLost_ErrData;
} ZCAN_CHANNEL_ERR_INFO;

typedef struct tagZCAN_CHANNEL_STATUS {
    BYTE errInterrupt;
    BYTE regMode;
    BYTE regStatus;
    BYTE regALCapture;
    BYTE regECCapture;
    BYTE regEWLimit;
    BYTE regRECounter;
    BYTE regTECounter;
    UINT Reserved;
} ZCAN_CHANNEL_STATUS;

// ---------------------------------------------------------------------------
// 收发数据结构
// ---------------------------------------------------------------------------
typedef struct tagZCAN_Transmit_Data {
    can_frame frame;
    UINT      transmit_type;    // 0:正常发送 1:单次发送 2:自发自收 3:单次自发自收
} ZCAN_Transmit_Data;

typedef struct tagZCAN_Receive_Data {
    can_frame frame;
    UINT64    timestamp;        // us，基于设备启动时间
} ZCAN_Receive_Data;

typedef struct tagZCAN_TransmitFD_Data {
    canfd_frame frame;
    UINT        transmit_type;
} ZCAN_TransmitFD_Data;

typedef struct tagZCAN_ReceiveFD_Data {
    canfd_frame frame;
    UINT64      timestamp;      // us
} ZCAN_ReceiveFD_Data;

// ---------------------------------------------------------------------------
// 兼容 ZLG ControlCAN 库的数据类型（VCI 接口）
// ---------------------------------------------------------------------------
typedef struct _VCI_BOARD_INFO {
    USHORT  hw_Version;
    USHORT  fw_Version;
    USHORT  dr_Version;
    USHORT  in_Version;
    USHORT  irq_Num;
    BYTE    can_Num;
    CHAR    str_Serial_Num[20];
    CHAR    str_hw_Type[40];
    USHORT  Reserved[4];
} VCI_BOARD_INFO, *PVCI_BOARD_INFO;

typedef struct _VCI_CAN_OBJ {
    UINT    ID;
    UINT    TimeStamp;      // 0.1ms
    BYTE    TimeFlag;
    BYTE    SendType;       // 0:正常发送 1:单次发送
    BYTE    RemoteFlag;     // 0:数据帧 1:远程帧
    BYTE    ExternFlag;     // 0:标准帧 1:扩展帧
    BYTE    DataLen;
    BYTE    Data[8];
    BYTE    Reserved[3];
} VCI_CAN_OBJ, *PVCI_CAN_OBJ;

typedef struct _VCI_CAN_STATUS {
    UCHAR   ErrInterrupt;
    UCHAR   regMode;
    UCHAR   regStatus;
    UCHAR   regALCapture;
    UCHAR   regECCapture;
    UCHAR   regEWLimit;
    UCHAR   regRECounter;
    UCHAR   regTECounter;
    DWORD   Reserved;
} VCI_CAN_STATUS, *PVCI_CAN_STATUS;

typedef struct _ERR_INFO {
    UINT    ErrCode;
    BYTE    Passive_ErrData[3];
    BYTE    ArLost_ErrData;
} VCI_ERR_INFO, *PVCI_ERR_INFO;

typedef struct _INIT_CONFIG {
    DWORD   AccCode;
    DWORD   AccMask;
    DWORD   Reserved;
    UCHAR   Filter;
    UCHAR   Timing0;
    UCHAR   Timing1;
    UCHAR   Mode;
} VCI_INIT_CONFIG, *PVCI_INIT_CONFIG;

typedef struct _VCI_FILTER_RECORD {
    DWORD   ExtFrame;   // 是否为扩展帧
    DWORD   Start;
    DWORD   End;
} VCI_FILTER_RECORD, *PVCI_FILTER_RECORD;

// ---------------------------------------------------------------------------
// IProperty：设备属性配置接口（GetIProperty 返回）
// ---------------------------------------------------------------------------
struct _Meta;
struct _Pair;
struct _Options;
struct _ConfigNode;

typedef struct _Meta       Meta;
typedef struct _Pair       Pair;
typedef struct _Options    Options;
typedef struct _ConfigNode ConfigNode;

struct _Options {
    const char* type;
    const char* value;
    const char* desc;
};

struct _Meta {
    const char*  type;
    const char*  desc;
    int          read_only;
    const char*  format;
    double       min_value;
    double       max_value;
    const char*  unit;
    double       delta;
    const char*  visible;
    const char*  enable;
    int          editable;
    Options**    options;
};

struct _Pair {
    const char* key;
    const char* value;
};

struct _ConfigNode {
    const char*  name;
    const char*  value;
    const char*  binding_value;
    const char*  path;
    Meta*        meta_info;
    ConfigNode** children;
    Pair**       attributes;
};

typedef const ConfigNode* (*GetPropertysFunc)();
typedef int  (*SetValueFunc)(const char* path, const char* value);   // 成功返回 1
typedef const char* (*GetValueFunc)(const char* path);               // 成功返回属性值

typedef struct tagIProperty {
    SetValueFunc     SetValue;
    GetValueFunc     GetValue;
    GetPropertysFunc GetPropertys;
} IProperty;

// ---------------------------------------------------------------------------
// 函数声明
// ---------------------------------------------------------------------------
#define FUNC_CALL __stdcall

typedef void* DEVICE_HANDLE;
typedef void* CHANNEL_HANDLE;

#ifdef __cplusplus
extern "C" {
#endif

DEVICE_HANDLE FUNC_CALL ZCAN_OpenDevice(UINT device_type, UINT device_index, UINT reserved);
UINT FUNC_CALL ZCAN_CloseDevice(DEVICE_HANDLE device_handle);
UINT FUNC_CALL ZCAN_GetDeviceInf(DEVICE_HANDLE device_handle, ZCAN_DEVICE_INFO* pInfo);
UINT FUNC_CALL ZCAN_IsDeviceOnLine(DEVICE_HANDLE device_handle);

CHANNEL_HANDLE FUNC_CALL ZCAN_InitCAN(DEVICE_HANDLE device_handle, UINT can_index, ZCAN_CHANNEL_INIT_CONFIG* pInitConfig);
UINT FUNC_CALL ZCAN_StartCAN(CHANNEL_HANDLE channel_handle);
UINT FUNC_CALL ZCAN_ResetCAN(CHANNEL_HANDLE channel_handle);
UINT FUNC_CALL ZCAN_ClearBuffer(CHANNEL_HANDLE channel_handle);
UINT FUNC_CALL ZCAN_ReadChannelErrInfo(CHANNEL_HANDLE channel_handle, ZCAN_CHANNEL_ERR_INFO* pErrInfo);
UINT FUNC_CALL ZCAN_ReadChannelStatus(CHANNEL_HANDLE channel_handle, ZCAN_CHANNEL_STATUS* pCANStatus);
UINT FUNC_CALL ZCAN_GetReceiveNum(CHANNEL_HANDLE channel_handle, BYTE type);  // type: TYPE_CAN / TYPE_CANFD
UINT FUNC_CALL ZCAN_Transmit(CHANNEL_HANDLE channel_handle, ZCAN_Transmit_Data* pTransmit, UINT len);
UINT FUNC_CALL ZCAN_Receive(CHANNEL_HANDLE channel_handle, ZCAN_Receive_Data* pReceive, UINT len, int wait_time);
UINT FUNC_CALL ZCAN_TransmitFD(CHANNEL_HANDLE channel_handle, ZCAN_TransmitFD_Data* pTransmit, UINT len);
UINT FUNC_CALL ZCAN_ReceiveFD(CHANNEL_HANDLE channel_handle, ZCAN_ReceiveFD_Data* pReceive, UINT len, int wait_time);

IProperty* FUNC_CALL GetIProperty(DEVICE_HANDLE device_handle);
UINT FUNC_CALL ReleaseIProperty(IProperty* pIProperty);

UINT FUNC_CALL ZCAN_SetAbitBaud(DEVICE_HANDLE device_handle, UINT can_index, UINT abitbaud);
UINT FUNC_CALL ZCAN_SetDbitBaud(DEVICE_HANDLE device_handle, UINT can_index, UINT dbitbaud);
UINT FUNC_CALL ZCAN_SetCANFDStandard(DEVICE_HANDLE device_handle, UINT can_index, UINT canfd_standard);
UINT FUNC_CALL ZCAN_SetResistanceEnable(DEVICE_HANDLE device_handle, UINT can_index, UINT enable);
UINT FUNC_CALL ZCAN_SetBaudRateCustom(DEVICE_HANDLE device_handle, UINT can_index, char* RateCustom);

UINT FUNC_CALL ZCAN_ClearFilter(CHANNEL_HANDLE channel_handle);
UINT FUNC_CALL ZCAN_AckFilter(CHANNEL_HANDLE channel_handle);
UINT FUNC_CALL ZCAN_SetFilterMode(CHANNEL_HANDLE channel_handle, UINT mode);       // 0:标准帧 1:扩展帧
UINT FUNC_CALL ZCAN_SetFilterStartID(CHANNEL_HANDLE channel_handle, UINT startID);
UINT FUNC_CALL ZCAN_SetFilterEndID(CHANNEL_HANDLE channel_handle, UINT EndID);

// 兼容 CANtest 等软件的 VCI 接口
DWORD __stdcall VCI_OpenDevice(DWORD DeviceType, DWORD DeviceInd, DWORD Reserved);
DWORD __stdcall VCI_CloseDevice(DWORD DeviceType, DWORD DeviceInd);
DWORD __stdcall VCI_InitCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_INIT_CONFIG pInitConfig);
DWORD __stdcall VCI_ReadBoardInfo(DWORD DeviceType, DWORD DeviceInd, PVCI_BOARD_INFO pInfo);
DWORD __stdcall VCI_ReadErrInfo(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_ERR_INFO pErrInfo);
DWORD __stdcall VCI_ReadCANStatus(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_STATUS pCANStatus);
DWORD __stdcall VCI_GetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);
DWORD __stdcall VCI_SetReference(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, DWORD RefType, PVOID pData);
ULONG __stdcall VCI_GetReceiveNum(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD __stdcall VCI_ClearBuffer(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD __stdcall VCI_StartCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
DWORD __stdcall VCI_ResetCAN(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd);
ULONG __stdcall VCI_Transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, DWORD Length);
ULONG __stdcall VCI_Receive(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime);

// 固件升级（函数签名以设备 SDK 为准）
DWORD __stdcall Firmware_Update(DWORD DeviceType, DWORD DeviceInd, const char* pFileName);

#ifdef __cplusplus
}
#endif

#endif // CONTROLCANFD_H
