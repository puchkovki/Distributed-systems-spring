#include <iostream>

using namespace std;

int main(void) 
{
    double a, b;
    cin >> a >> b;
    cout << a/b<< "\n";
    return 0;
}

/*
set terminal png size 1024, 720
set xlabel "Number of processes"
set ylabel "Acceleration"
set xrange [1:24]
set output 'name.png'
plot "№.txt" with lines


scp -P 50506 solution.c puchkovki@remote.vdi.mipt.ru:~
ssh -p 50506 puchkovki@remote.vdi.mipt.ru
ssh s77403@head.vdi.mipt.ru*/