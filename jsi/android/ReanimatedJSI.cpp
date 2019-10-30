// put this file to node_modules/react-native/ReactAndroid/src/main/java/com/facebook/react/jscexecutor

#include <fb/fbjni.h>
#include <jsi/JSIDynamic.h>
#include <react/jni/ReadableNativeMap.h>
#include <fb/log.h>
#include <jsi/jsi.h>
#include <jni.h>
#include <android/log.h>
#include "./ReanimatedJSI.h"

#define APPNAME "ReanimatedJSI"

void JNICALL Java_com_swmansion_reanimated_ReanimatedModule_installJSI(
  JNIEnv *env,
  jobject thiz,
  jlong runtimePtr
) {
  auto &runtime = *(facebook::jsi::Runtime *)runtimePtr;

  auto clazz = env->FindClass("com/swmansion/reanimated/ReanimatedModule");
  auto createNode = env->GetMethodID(clazz, "createNodeJSI", "(Lcom/facebook/react/bridge/ReadableMap;)V");
  auto dropNode = env->GetMethodID(clazz, "dropNode", "(I)V");
  auto connectNodes = env->GetMethodID(clazz, "connectNodes", "(II)V");
  auto disconnectNodes = env->GetMethodID(clazz, "disconnectNodes", "(II)V");
  auto connectNodeToView = env->GetMethodID(clazz, "connectNodeToView", "(II)V");
  auto disconnectNodeFromView = env->GetMethodID(clazz, "disconnectNodeFromView", "(II)V");
  auto attachEvent = env->GetMethodID(clazz, "attachEvent", "(ILjava/lang/String;I)V");
  auto detachEvent = env->GetMethodID(clazz, "detachEvent", "(ILjava/lang/String;I)V");
  auto getValue = env->GetMethodID(clazz, "getValue", "(ILcom/swmansion/reanimated/Callback;)V");

  auto module = std::make_shared<ReanimatedJSI>(
    // clazz,
    thiz,
    createNode,
    dropNode,
    connectNodes,
    disconnectNodes,
    connectNodeToView,
    disconnectNodeFromView,
    attachEvent,
    detachEvent,
    getValue
  );

  ReanimatedJSI::install(runtime, module);
}

using namespace facebook;
using namespace facebook::jni;
using namespace facebook::react;

void JNICALL Java_com_swmansion_reanimated_Callback_nativeInvoke(
  JNIEnv *env,
  jobject thiz,
  jlong runtimePtr,
  jlong callbackPtr,
  jstring value
) {
  auto &runtime = *(jsi::Runtime *)runtimePtr;
  auto &fn = *(jsi::Function *)callbackPtr;

  const char *str = env->GetStringUTFChars(value, nullptr);

  fn.call(runtime, str, 1);
}


inline local_ref<ReadableMap::javaobject> castReadableMap(
    local_ref<ReadableNativeMap::javaobject> nativeMap
) {
  return make_local(reinterpret_cast<ReadableMap::javaobject>(nativeMap.get()));
}

inline local_ref<ReadableArray::javaobject> castReadableArray(
  local_ref<ReadableNativeArray::javaobject> nativeArray
) {
  return make_local(reinterpret_cast<ReadableArray::javaobject>(nativeArray.get()));
}

jintArray createJIntArray(JNIEnv *env, jsi::Runtime &runtime, jsi::Value arg) {
    auto arr = arg.getObject(runtime).asArray(runtime).getArray(runtime);
    // __android_log_print(ANDROID_LOG_DEBUG, APPNAME, "Number is %f", number);

    auto arrayLength = arr.length(runtime);

    int nativeArr[arrayLength];
    for (unsigned int i = 0; i < arrayLength; i++) {
        auto value = (jint)arr.getValueAtIndex(runtime, i).asNumber();
        nativeArr[i] = value;
    }

    auto javaArray = env->NewIntArray(arrayLength);

    env->SetIntArrayRegion(javaArray, 0, arrayLength, nativeArr);

    return javaArray;
}

ReanimatedJSI::ReanimatedJSI(
  // jclass moduleClass,
  jobject moduleObject,
  jmethodID createNode,
  jmethodID dropNode,
  jmethodID connectNodes,
  jmethodID disconnectNodes,
  jmethodID connectNodeToView,
  jmethodID disconnectNodeFromView,
  jmethodID attachEvent,
  jmethodID detachEvent,
  jmethodID getValue
): 
  // _moduleClass(moduleClass),
  _moduleObject(moduleObject),
  _createNode(createNode),
  _dropNode(dropNode),
  _connectNodes(connectNodes),
  _disconnectNodes(disconnectNodes),
  _connectNodeToView(connectNodeToView),
  _disconnectNodeFromView(disconnectNodeFromView),
  _attachEvent(attachEvent),
  _detachEvent(detachEvent),
  _getValue(getValue)
{}

void ReanimatedJSI::install(
  jsi::Runtime &runtime,
  const std::shared_ptr<ReanimatedJSI> module
) {
  auto name = "NativeReanimated";
  auto object = jsi::Object::createFromHostObject(runtime, module);

  runtime.global().setProperty(runtime, name, std::move(object));
}

jsi::Value ReanimatedJSI::get(
  jsi::Runtime &runtime,
  const jsi::PropNameID &name
) {
  auto methodName = name.utf8(runtime);

  if (methodName == "getValue") {
    auto &method = _getValue;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      auto nodeId = (jint)arguments[0].asNumber();
      auto fn = arguments[1].asObject(runtime).asFunction(runtime);

      jlong fnPtr = (jlong) &fn;
      jlong runtimePtr = (jlong) &runtime;

      jclass callbackClass = env->FindClass("com/swmansion/reanimated/Callback");
      jmethodID callbackConstructor = env->GetMethodID(callbackClass, "<init>", "(JJ)V");
      jobject callback = env->NewObject(callbackClass, callbackConstructor, runtimePtr, fnPtr);

      // this line throws "JNI DETECTED ERROR IN APPLICATION: use of invalid jobject 0x7ff25f9e14"
      env->CallVoidMethod(moduleObject, method, nodeId, callback);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "createNode") {
    auto &method = _createNode;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      auto dynamicMap = jsi::dynamicFromValue(runtime, arguments[0]);
      local_ref<ReadableMap::javaobject> readableMap =
        castReadableMap(ReadableNativeMap::newObjectCxxArgs(dynamicMap));

      env->CallVoidMethod(moduleObject, method, readableMap.get());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "createNodeOperator") {
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();
  
      auto nodeId = (jint)arguments[0].asNumber();
      auto op = make_jstring(arguments[1].asString(runtime).utf8(runtime));
      auto arr = createJIntArray(env, runtime, &arguments[2]);

      auto method = env->GetMethodID(clazz, "createNodeOperator", "(ILjava/lang/String;[I)V");
      env->CallVoidMethod(moduleObject, method, nodeId, op, arr);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "createNodeCallFunc") {
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();
  
      auto nodeId = (jint)arguments[0].asNumber();
      auto what = (jint)arguments[1].asNumber();

      auto args = createJIntArray(env, runtime, &arguments[2]);
      auto params = createJIntArray(env, runtime, &arguments[3]);

      auto method = env->GetMethodID(clazz, "createNodeCallFunc", "(II[I[I)V");
      env->CallVoidMethod(moduleObject, method, nodeId, what, args, params);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "createNodeFunction") {
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();
  
      auto nodeId = (jint)arguments[0].asNumber();
      auto what = (jint)arguments[1].asNumber();

      auto method = env->GetMethodID(clazz, "createNodeFunction", "(II)V");
      env->CallVoidMethod(moduleObject, method, nodeId, what);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "createNodeParam") {
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();
  
      auto nodeId = (jint)arguments[0].asNumber();

      auto method = env->GetMethodID(clazz, "createNodeParam", "(I)V");
      env->CallVoidMethod(moduleObject, method, nodeId);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "dropNode") {
    auto &method = _dropNode;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 1, callback);
  }

  if (methodName == "connectNodes") {
    auto &method = _connectNodes;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), (jint)arguments[1].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 2, callback);
  }

  if (methodName == "disconnectNodes") {
    auto &method = _disconnectNodes;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), (jint)arguments[1].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 2, callback);
  }

  if (methodName == "connectNodeToView") {
    auto &method = _connectNodeToView;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), (jint)arguments[1].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 2, callback);
  }

  if (methodName == "disconnectNodeFromView") {
    auto &method = _disconnectNodeFromView;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), (jint)arguments[1].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 2, callback);
  }

  if (methodName == "attachEvent") {
    auto &method = _attachEvent;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      auto eventName = make_jstring(arguments[1].asString(runtime).utf8(runtime));

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), eventName.get(), (jint)arguments[2].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 3, callback);
  }

  if (methodName == "detachEvent") {
    auto &method = _detachEvent;
    auto &moduleObject = _moduleObject;

    auto callback = [moduleObject, method](
      jsi::Runtime &runtime,
      const jsi::Value &thisValue,
      const jsi::Value *arguments,
      size_t count
    ) -> jsi::Value {
      auto env = Environment::current();

      auto eventName = make_jstring(arguments[1].asString(runtime).utf8(runtime));

      env->CallVoidMethod(moduleObject, method, (jint)arguments[0].asNumber(), eventName.get(), (jint)arguments[2].asNumber());

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 3, callback);
  }

  return jsi::Value::undefined();
}
