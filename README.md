This is a Linux port of Java injector https://github.com/TheQmaks/JavaInjector.

This tool inject your jar file and allocates object of class, which name must be written in zip comment.
Entry point of your main-class will be constructor.
After building you classes, you should put them into archive using WinRAR of other archivator which can change archive comment.
You should write main class in comment as single line.
Example:
Put class with name "Main" inside package "test", constructor will be your entry-point, e.g
"public Main() { Your awesome code }", so you should set comment in your archive with classes to "test.Main" without quotes. 

How to inject on linux? I use this https://github.com/kubo/injector.  
How to build?   
git clone https://github.com/YuraLink/JavaInjector.git  
cd JavaInjector  
mkdir build  
cd build  
cmake ..  
make  

Use: ./injector -p (minecraft pid process) /path/to/libJavaInjector.so  
After the file selection window pops up, you need to select your cheat.


