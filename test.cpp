#include <iostream>
using namespace std;
#include <QDebug>

int main(void)
{
    cout << "__cplusplus " << __cplusplus << std::endl;
    cout << "__VERSION__ " << __VERSION__ << std::endl;
    cout << "cout ok\n";
    qDebug() << "qdebug ok";
    vector<int> v = { 1, 3, 2 };
    cout << v[0] << endl;
    QVector<int> vv = { 3, 2, 1 };
    qDebug() << vv;
    return 0;
}
