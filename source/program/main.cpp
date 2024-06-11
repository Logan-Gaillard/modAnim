#include "lib.hpp"
#include "patches.hpp"
#include "logger/Logger.hpp"

#include "sead/basis/seadRawPrint.h"
#include "sead/resource/seadResourceMgr.h"
#include "sead/heap/seadHeapMgr.h"
#include "sead/devenv/seadDebugFontMgrNvn.h"
#include "sead/gfx/seadTextWriter.h"
#include "sead/gfx/seadViewport.h"
#include "sead/time/seadTickSpan.h"

#include <cmath>

#include "al/util.hpp"
#include "game/System/GameSystem.h"
#include "game/System/Application.h"
#include "game/HakoniwaSequence/HakoniwaSequence.h"
#include "al/actor/LiveActorFlag.h"
#include "rs/util.hpp"

#include "sead/gfx/seadPrimitiveRenderer.h"

#include "al/util/ControllerUtil.h"
#include "game/Player/PlayerInput.h"

static const char *DBG_FONT_PATH = "DebugData/Font/nvn_font_jis1.ntx";
static const char *DBG_SHADER_PATH = "DebugData/Font/nvn_font_shader_jis1.bin";
static const char *DBG_TBL_PATH = "DebugData/Font/nvn_font_jis1_tbl.bin";

sead::TextWriter *gTextWriter;

float playerHakoniwaConsts[2] = {0.0f, 0.0f};
// mNormalMinSpeed, mNormalMaxSpeed

HOOK_DEFINE_REPLACE(ReplaceSeadPrint) {
    static void Callback(const char *format, ...) {
        va_list args;
        va_start(args, format);
        Logger::log(format, args);
        va_end(args);
    }
};

HOOK_DEFINE_TRAMPOLINE(GameSystemInit) {
    static void Callback(GameSystem *thisPtr) {

        sead::Heap *curHeap = sead::HeapMgr::instance()->getCurrentHeap();

        sead::DebugFontMgrJis1Nvn::createInstance(curHeap);

        if (al::isExistFile(DBG_SHADER_PATH) && al::isExistFile(DBG_FONT_PATH) && al::isExistFile(DBG_TBL_PATH)) {
            sead::DebugFontMgrJis1Nvn::sInstance->initialize(curHeap, DBG_SHADER_PATH, DBG_FONT_PATH, DBG_TBL_PATH, 0x100000);
        }

        sead::TextWriter::setDefaultFont(sead::DebugFontMgrJis1Nvn::sInstance);

        al::GameDrawInfo *drawInfo = Application::instance()->mDrawInfo;

        agl::DrawContext *context = drawInfo->mDrawContext;
        agl::RenderBuffer *renderBuffer = drawInfo->mFirstRenderBuffer;

        auto *viewport = new sead::Viewport(*renderBuffer);

        gTextWriter = new sead::TextWriter(context, viewport);

        sead::TextWriter::setupGraphics(context);

        gTextWriter->mColor = sead::Color4f(1.f, 1.f, 1.f, 0.8f);

        Orig(thisPtr);

    }
};

HOOK_DEFINE_TRAMPOLINE(DrawDebugMenu) {
    static void Callback(HakoniwaSequence *thisPtr) {

        Orig(thisPtr);

        gTextWriter->beginDraw();

        gTextWriter->setCursorFromTopLeft(sead::Vector2f(10.f, 10.f));
        gTextWriter->printf("FPS: %d\n", static_cast<int>(std::round(Application::instance()->mFramework->calcFps())));

        gTextWriter->endDraw();
    }
};

HOOK_DEFINE_TRAMPOLINE(HakoniwaSequenceUpdate) {
    static void Callback(HakoniwaSequence *thisPtr) {
        Orig(thisPtr);

        al::PlayerHolder* pHolder = al::getScenePlayerHolder(thisPtr->getCurrentScene());
        PlayerActorHakoniwa* p1 = pHolder->tryGetPlayer(0);


        if(al::isPadTriggerPressLeftStick(-1)) {
          p1->mPlayerAnimator->startSubAnim("DashFast");

          //p1->mPlayerConst->mNormalMinSpeed = 

          //1s cooldown cpp and stop anim
          sead::TickSpan span = sead::TickSpan::makeFromMilliSeconds(500);
          sead::Thread::sleep(span);

          p1->mPlayerAnimator->endSubAnim();

        }

    }
};

extern "C" void exl_main(void *x0, void *x1) {
    /* Setup hooking enviroment. */
    exl::hook::Initialize();
#ifdef SMO_DEBUG
    const u16 PORT = 3080;
#ifdef ABORT_IF_NO_SERVER
    R_ABORT_UNLESS(Logger::instance().init(LOGGER_IP, PORT).value)
#else
    Logger::instance().init(LOGGER_IP, PORT);
#endif
    GameSystemInit::InstallAtOffset(0x535850);
    // Debug Text Writer Drawing
    DrawDebugMenu::InstallAtOffset(0x50F1D8);

    runDebugCodePatches();
#endif
    runCodePatches();

    // Sead Debugging Overriding
    ReplaceSeadPrint::InstallAtOffset(0xB59E28);
    HakoniwaSequenceUpdate::InstallAtOffset(0x50F030);
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}
