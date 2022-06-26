/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/window.h>
#include <android/log.h>
#include <android_native_app_glue.h>


extern "C" {
#include "mogl22d/mogl22d.h"
#include "fonts/fonts.h"
#include <gc.h>
  
#include "limo/limo.h"

#include "_program.h"       // this is the limo-program

  /// text.c
  extern mogl22d_surface *mogl22d_textms;
  void mogl22d_font_plot_function(int x, int y, int c);
  void mogl22d_filled_box_function(int x1, int y1, int x2, int y2, int c);
  font myFont;  
}

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
  struct android_app* app;
  
  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;
  
  int animating;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;
  
  mogl22d_surface *m_surface;
  limo_data *env;
  
  struct saved_state state;
};

struct engine *global_engine;

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, nullptr, nullptr);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return -1;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    context = eglCreateContext(display, config, nullptr, nullptr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;


    LOGI("main.cpp - Line: 154\n");
    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }
    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    ///// MO INIT MY MOGL22D-Surface here    

    myFont = FontLoadFromArray(dosbox_8x16_fnt, dosbox_8x16_fnt_len);
    FontPlotFunction = mogl22d_font_plot_function;
    FontFilledBoxFunction = mogl22d_filled_box_function;
  
    engine->m_surface = mogl22d_init(engine->width, engine->height);
    mogl22d_textms = engine->m_surface;

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {

  if (engine->display == nullptr) {
      // No display.
      return;
  }
  
  // MO: we comment this out, because we're doing our own stuff!
  // // Just fill the screen with a color.
  // glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
  //              ((float)engine->state.y)/engine->height, 1);
  // glClear(GL_COLOR_BUFFER_BIT);

  mogl22d_draw_x(engine->m_surface);
  
  eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
  limo_data *ld_type;
  limo_data *ld_x;
  limo_data *ld_y;
  
  auto* engine = (struct engine*)app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    switch (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_DOWN:
      LOGI("input_handler: AMOTION_EVENT_ACTION_DOWN");
      ld_type = make_sym((char *)"AMOTION_EVENT_ACTION_DOWN");
      ld_x = make_number_from_long_long(engine->state.x = AMotionEvent_getX(event, 0));
      ld_y = make_number_from_long_long(engine->state.y = AMotionEvent_getY(event, 0));
      setf(engine->env, make_sym((char *)"_ANDROID-LAST-EVENT"), make_list(1, ld_type, ld_x, ld_y, NULL));
      break;
      
    case AMOTION_EVENT_ACTION_UP:
      LOGI("input_handler: AMOTION_EVENT_ACTION_UP");
      ld_type = make_sym((char *)"AMOTION_EVENT_ACTION_UP");
      ld_x = make_number_from_long_long(engine->state.x = AMotionEvent_getX(event, 0));
      ld_y = make_number_from_long_long(engine->state.y = AMotionEvent_getY(event, 0));
      setf(engine->env, make_sym((char *)"_ANDROID-LAST-EVENT"), make_list(1, ld_type, ld_x, ld_y, NULL));
      break;
    }
    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
	  LOGI("APP_CMD_SAVE_STATE");
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
	  LOGI("APP_CMD_INIT_WINDOW");
	  // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
	  LOGI("APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
	  LOGI("APP_CMD_GAINED_FOCUS");
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
	  LOGI("APP_CMD_LOST_FOCUS");
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
        default:
	  LOGI("APP_CMD WEISSNICH");
            break;
    }
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(android_app* app) {

  if(!app)
    return nullptr;

  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
      dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");
    auto packageName= (jstring)env->CallObjectMethod(app->activity->clazz,
                                                        midGetPackageName);

    const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    app->activity->vm->DetachCurrentThread();
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }

  typedef ASensorManager *(*PF_GETINSTANCE)();
  auto getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  dlclose(androidHandle);

  return getInstanceFunc();
}


extern "C" {

  BUILTINFUN(builtin_mogl22d_inited)
  {
    if (global_engine->display == nullptr) {
      // No display.
      return nil;
    }
    else {
      return sym_true;
    }
  }

  BUILTINFUN(builtin_mogl22d_clear)
  {
    mogl22d_clear(global_engine->m_surface);
    return nil;
  }

  BUILTINFUN(builtin_mogl22d_gotoxy)
  {    
    REQUIRE_ARGC_FUN((char *)"mogl22d_gotoxy", 2);
    REQUIRE_TYPE((char *)"mogl22d_gotoxy", argv[0], limo_TYPE_GMPQ);
    REQUIRE_TYPE((char *)"mogl22d_gotoxy", argv[1], limo_TYPE_GMPQ);
    
    FontGotoXY(GETINTFROMMPQ(argv[0]), GETINTFROMMPQ(argv[1]));
    return nil;
  }
  
  BUILTINFUN(builtin_mogl22d_setcolor)
  {
    REQUIRE_ARGC_FUN((char *)"mogl22d_setcolor", 2);
    REQUIRE_TYPE((char *)"mogl22d_setcolor", argv[0], limo_TYPE_GMPQ);
    REQUIRE_TYPE((char *)"mogl22d_setcolor", argv[1], limo_TYPE_GMPQ);
  
    FontSetColor(GETINTFROMMPQ(argv[0]) % 16,
		 GETINTFROMMPQ(argv[1]) % 16);

    return nil;
  }
  
  BUILTINFUN(builtin_mogl22d_setsize)
  {
    REQUIRE_ARGC_FUN((char *)"mogl22d_setsize", 1);
    REQUIRE_TYPE((char *)"mogl22d_setsize", argv[0], limo_TYPE_GMPQ);
    FontSetSize(GETINTFROMMPQ(argv[0]));
    return nil;
  }

  BUILTINFUN(builtin_mogl22d_getglyphsize)
  {
    int size = FontGetSize();
    int ewidth;
    ewidth = size * myFont->pics[0].width / myFont->height;
    return make_cons(make_number_from_long_long(ewidth), make_cons(make_number_from_long_long(size), nil));
  }

  BUILTINFUN(builtin_mogl22d_getdisplaysize)
  {
    return make_cons(make_number_from_long_long(global_engine->m_surface->width),
		     make_cons(make_number_from_long_long(global_engine->m_surface->height),
			       nil));
  }
  
  BUILTINFUN(builtin_mogl22d_write)
  {
    REQUIRE_ARGC_FUN((char *)"mogl22d_write", 1);
    REQUIRE_TYPE((char *)"mogl22d_write", argv[0], limo_TYPE_STRING);
    
    FontWrite(myFont, argv[0]->d_string);
    return nil;
  }

  BUILTINFUN(builtin_mogl22d_flip)
  {
    mogl22d_flip(global_engine->m_surface);
    engine_draw_frame(global_engine);
    return nil;
  }

  BUILTIN(builtin_android_poll_events)
  {
    int ident;
    int events;
    struct android_poll_source* source;

    REQUIRE_ARGC((char *)"android_poll_events", 1);

    setf(env, make_sym((char *)"_ANDROID-LAST-EVENT"), nil);
    
    if ((ident=ALooper_pollOnce(is_nil(eval(FIRST_ARG, env)) ? 0 : -1, nullptr, &events,
				(void**)&source)) >= 0) {
      
      // Process this event.
      if (source != nullptr) {
	// source->process(state, source);
	source->process(global_engine->app, source);
      }      
      
      // Check if we are exiting.
      if (global_engine->app->destroyRequested != 0) {
	engine_term_display(global_engine);
	limo_error((char *)"display destroyed");
      }

      return sym_true;   /// TODO: how do events work?!
    }
    else {
      return nil;
    }
    
  }

  BUILTINFUN(builtin_android_logi)
  {
    REQUIRE_ARGC_FUN((char *)"android-logi", 1);
    REQUIRE_TYPE((char *)"android-logi", argv[0], limo_TYPE_STRING);
    
    LOGI("%s\n", argv[0]->d_string);
    return nil;
  }

  BUILTINFUN(builtin_android_abort)
  {
    LOGI("ANDROID-ABORT Called");
    __builtin_trap();
  }

  limo_data *init_android_limo(struct android_app* state)
  {
    limo_data *env;
    env = limo_init(0, NULL);
    setq(env, make_sym((char *)"mogl22d-inited"), make_builtinfun(builtin_mogl22d_inited));;
    setq(env, make_sym((char *)"mogl22d-clear"), make_builtinfun(builtin_mogl22d_clear));
    setq(env, make_sym((char *)"mogl22d-write"), make_builtinfun(builtin_mogl22d_write));
    setq(env, make_sym((char *)"mogl22d-gotoxy"), make_builtinfun(builtin_mogl22d_gotoxy));
    setq(env, make_sym((char *)"mogl22d-setcolor"), make_builtinfun(builtin_mogl22d_setcolor));
    setq(env, make_sym((char *)"mogl22d-setsize"), make_builtinfun(builtin_mogl22d_setsize));
    setq(env, make_sym((char *)"mogl22d-flip"), make_builtinfun(builtin_mogl22d_flip));
    setq(env, make_sym((char *)"mogl22d-getglyphsize"), make_builtinfun(builtin_mogl22d_getglyphsize));
    setq(env, make_sym((char *)"mogl22d-getdisplaysize"), make_builtinfun(builtin_mogl22d_getdisplaysize));
    setq(env, make_sym((char *)"android-poll-events"), make_builtin(builtin_android_poll_events));
    setq(env, make_sym((char *)"android-logi"), make_builtinfun(builtin_android_logi));
    setq(env, make_sym((char *)"android-abort"), make_builtinfun(builtin_android_abort));
    
    setq(env, make_sym((char *)"android-internal-data-path"), make_string((char *)state->activity->internalDataPath));
    
    setq(env, make_sym((char *)"_ANDROID-LAST-EVENT"), nil);
    return env;
  }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {

  ANativeActivity_setWindowFlags(state->activity, AWINDOW_FLAG_FORCE_NOT_FULLSCREEN, AWINDOW_FLAG_FULLSCREEN);
  
    struct engine engine{};

    limo_data *env;
    reader_stream *rs;

    global_engine = &engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    // TODO: we don't really need this now.
    engine.sensorManager = AcquireASensorManagerInstance(state);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
                                        engine.sensorManager,
                                        ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
                                    engine.sensorManager,
                                    state->looper, LOOPER_ID_USER,
                                    nullptr, nullptr);

    if (state->savedState != nullptr) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    env = init_android_limo(state);
    engine.env = env;

    rs = limo_rs_from_string((char *)_program_limo, env);

    while (!limo_eof(rs)) {
      if (NULL==try_catch(reader(rs), env)) {
	LOGI("LIMO: Exception Caught!\n");
	print_stacktrace(stacktrace);
	writer(pk_exception_get());
	printf("\n");
	__builtin_trap();
      }
    }

    LOGI("That's all, folks!\n");
    __builtin_trap();

    // TODO: und dann noch irgendein Proof of Concept schreiben

}
//END_INCLUDE(all)
