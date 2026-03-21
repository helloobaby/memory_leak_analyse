之前有部分分析的步骤在readme.md.old。

当时windbg对32位程序的内存泄漏的分析不是很好，基本等于没分析出来，但是目前工作遇到的客户端基本都是32位的。



最近发现了一个用etl用的很厉害的人的博客，学习一波用etl分析堆内存泄漏。

https://randomascii.wordpress.com/2015/04/27/etw-heap-tracingevery-allocation-recorded/



![image-20260322000714786](images/readme/image-20260322000714786.png)

![image-20260322000726543](images/readme/image-20260322000726543.png)、

我按照博客里的设置，然后抓ETL，分析ETL。



![image-20260322001026392](images/readme/image-20260322001026392.png)



用WPA打开后，然后双击右边的这个的view，这是作者写的view profile相当于是，不是wpa自带的界面设置。

![image-20260322001948515](images/readme/image-20260322001948515.png)具体看这两个。

Leak_ApiInternalBuffer中间调用GetAdaptersAddresses和HeapAlloc。

调用HeapAlloc的内存207次没有释放，GetAdaptersAddresses中间的1951次是系统分配的没有释放。我们只需要关注自己释放的没分配就行了。不过这个Demo太简单了，不确定实际分析中有没有用。





