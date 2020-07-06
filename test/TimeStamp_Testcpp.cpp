
#include "./base/AsyncLog.h"
#include "./base/Timestamp.h"

int main()
{
    CAsyncLog::init();
    Timestamp a(2);
    Timestamp b(1);
    Timestamp c = Timestamp::addTime(a, b);

    Timestamp now(Timestamp::now());
    LOGI("%s + %s = %s", a.toString().c_str(), b.toString().c_str(), c.toString().c_str());
    LOGI("%s", now.toString().c_str());
    CAsyncLog::uninit();
    return 0;
}