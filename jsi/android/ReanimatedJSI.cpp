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
  auto getValue = env->GetMethodID(clazz, "getValue", "(ILcom/facebook/react/bridge/Callback;)V");

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

      auto nodeId = (jint)arguments[0];
      auto fn = arguments[1].asObject(runtime).asFunction(runtime);

      auto callback = nullptr; // TODO:  create java callback

      env->CallVoidMethod(moduleObject, method, nodeId, callback);

      return jsi::Value::undefined();
    };

    return jsi::Function::createFromHostFunction(runtime, name, 2, callback);
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