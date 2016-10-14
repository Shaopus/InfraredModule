
# [**Infrared Module**](https://github.com/Shaopus/InfraredModule/)

* Author  : [ShaoPu](https://github.com/Shaopus)
* Version : v1.0
* Update  : 2016/10/14

Description
========
Infrared Module是基于红外上进行二次开发的无线设备，由STM8L151G4U(控制器)、CC1101(无线模块433M)等组成，使用外部12V电源供电，主要实现安防监控(需要与[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)进行配对)，整体体积15 * 30* 5 mm。

Hardware
========
* 控制器　  : STM8L151G4U
* 无线芯片  : CC1101
* 外接介面  : 1 x KEY
* PCB 尺寸 : 15 x 30 x 1 mm
* 設計軟體  : [Altium Designer 09](http://www.altium.com/en/products/altium-designer)

Software
========

该设备采用433M通信频率，无线芯片采用CC1101。考虑到电池使用寿命及报警器的工作特点，传感器类终端设备采用单向通信，发送数据只有一种帧结构，包含报警、电池欠压和设备状态信息。帧格式如下所示：

| 同步码  | 帧长度  | 控制码  |    产品序列号     |    模式    |
| :--: | :--: | :--: | :----------: | :------: |
| 8799 |  08  |  00  | 301234567890 | 00/F0/08 |

该设备与[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)工作流程：

1. 该设备发向后台的信息，[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)按照信息的含义完成相应的功能，并且都可作为心跳，表示终端在线

2. 该设备发生报警需要连续发送**二次**报警信息给[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)，两次发送数据间隔随机200-500毫秒

3. 该设备报警发送F0，[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)向后台发送告警信息，并提示有人闯入。

4. 该设备空闲超过**3小时**(即**心跳**)需要发送工作状态[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)

5. 该设备空闲超过**12小时**(即电压检测)，一天检测两次，即每4次心跳，需检测，如果电池欠压则需叠加电池欠压信息(08)发送工作状态[安防控制盒](https://github.com/Shaopus/CloudSecurityBox)

6. 该设备需在第一次上电时需检测电量，每次报警被触发时，不带欠压告警

7. 该设备需在特定Flash地址写入12位序列号，每个设备的序列号不能重号

   ​

   ​


