# DispatchTaskQueue
thread pool 
线程池，TaskQueue
实现功能有
- 在线程池中同步调用sync；
- 异步调用async
- 异步延迟调用
类似wpf的dispatcher，mac的dispatcher
功能主要用于实现任务的跨线程调用

# 构建项目

1. 此项目构建依赖cmake,python
2. 各平台构建
- windws   构建vs工程
 ```
 python bootstrap.py win
 ```
- linux  可支持clion
```shell
python bootstrap.py linux
cd build/linux
make
```
- mac 
```
python bootstrap.py osx
```
# 如何在你的项目中使用
1. 将源文件与头文件直接包含在你的项目中
2. 修改此cmake 生成库文件
