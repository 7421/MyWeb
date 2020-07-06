#include "./base/AsyncLog.h"

int main()
{
    CAsyncLog::init();
    LOGI("hello from MyWeb!\n");
    LOGW("hello from MyWeb!\n");
    LOGE("hello from MyWeb!\n");
    CAsyncLog::uninit();
    return 0;
}