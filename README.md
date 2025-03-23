# StreamingMedia

项目架构图如fig.1.所示

<img width="904" alt="image" src="https://github.com/user-attachments/assets/c1621d8a-7474-4465-84e2-8c01d3634a18" />

figrue.1. 项目架构图

##### **移动端 (Android)**

- 通过 Camera2 API、AudioRecord 等采集视频与音频数据，传递给 **Android适配层 (JNI)**。
- 适配层调用 **核心库 (C++层)** 进行图像滤镜（OpenCV）、音频编解码（FFmpeg）等处理，并通过 **跨终端通信模块(移动端)** 与桌面端通信。
- 最终在 **移动端UI (OpenGL ES, SurfaceView)** 进行渲染或播放处理后的数据。

##### **桌面端 (macOS / Windows等)**

- 通过摄像头、麦克风或其他接口采集数据，传给 **桌面端适配层 (Qt/C++层)**。
- 适配层调用同一个 **核心库 (C++层)** 进行多媒体处理，并通过 **跨终端通信模块(桌面端)** 与移动端进行数据互传。
- 最终在 **桌面端UI (Qt/SDL, OpenGL等)** 显示或播放处理后的内容。

##### **核心库 (C++层)**

- 统一实现多媒体处理与网络传输逻辑，包含对 OpenCV、FFmpeg、Socket 等库的集成。
- 通过跨终端通信模块（在移动端和桌面端各自实例）实现网络数据互通。

##### **跨终端通信**

- 采用 Socket 实现实时通信。
