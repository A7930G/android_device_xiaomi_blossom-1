#define LOG_TAG "powerhal-libperfmgr"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_ibinder_platform.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <perfmgr/HintManager.h>

#include <thread>

#include "Power.h"
#include "PowerExt.h"
#include "PowerSessionManager.h"

using aidl::google::hardware::power::impl::pixel::Power;
using aidl::google::hardware::power::impl::pixel::PowerExt;
using aidl::google::hardware::power::impl::pixel::PowerHintMonitor;
using aidl::google::hardware::power::impl::pixel::PowerSessionManager;
using ::android::perfmgr::HintManager;

constexpr std::string_view kPowerHalInitProp("vendor.powerhal.init");

int main() {
    std::shared_ptr<HintManager> hm(HintManager::GetInstance());
    if (!hm) {
        LOG(FATAL) << "HintManager Init failed";
    }

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    // core service
    std::shared_ptr<Power> pw = ndk::SharedRefBase::make<Power>();
    ndk::SpAIBinder pwBinder = pw->asBinder();
    AIBinder_setMinSchedulerPolicy(pwBinder.get(), SCHED_NORMAL, -20);

    // extension service
    std::shared_ptr<PowerExt> pwExt = ndk::SharedRefBase::make<PowerExt>();
    auto pwExtBinder = pwExt->asBinder();
    AIBinder_setMinSchedulerPolicy(pwExtBinder.get(), SCHED_NORMAL, -20);

    // attach the extension to the same binder we will be registering
    CHECK(STATUS_OK == AIBinder_setExtension(pwBinder.get(), pwExt->asBinder().get()));

    const std::string instance = std::string() + Power::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(pw->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);
    LOG(INFO) << "Lineage Power HAL AIDL Service with Extension is started.";

    if (HintManager::GetInstance()->GetAdpfProfile()) {
        PowerHintMonitor::getInstance()->start();
    }

    std::thread initThread([&]() {
        ::android::base::WaitForProperty(kPowerHalInitProp.data(), "1");
        HintManager::GetInstance()->Start();
    });
    initThread.detach();

    ABinderProcess_joinThreadPool();

    // should not reach
    LOG(ERROR) << "Lineage Power HAL AIDL Service with Extension just died.";
    return EXIT_FAILURE;
}
