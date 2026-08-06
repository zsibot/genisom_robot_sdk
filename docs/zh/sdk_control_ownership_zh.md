## SDK与APP控制权问题详解

**原则**

**1.APP端被允许抢SDK端的控制权；**

**2.SDK端不允许抢APP端的控制权；**

### 情况1：APP先连接，SDK后连接。控制权在 APP 端，SDK 无法控制。

![图片](images/APP-Control_zh.png)

### 情况2：SDK先连接，APP后连接。控制权在 SDK 端，APP 无法控制，但 APP 可以强行抢夺 SDK 控制权。

![图片](images/SDK-Control_zh.png)