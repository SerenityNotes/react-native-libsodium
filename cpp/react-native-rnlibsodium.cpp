// import our header file to implement the `installRnlibsodium` and `cleanUpRnlibsodium` functions
#include "react-native-rnlibsodium.h"
// useful functions manipulate strings in C++
#include <sstream>
// libsodium
#include "sodium.h"

// syntactic sugar around the JSI objects. ex. call: jsi::Function
using namespace facebook;

// get the runtime and create native functions
void installRnlibsodium(jsi::Runtime &jsiRuntime)
{
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_secretbox_KEYBYTES", (int)crypto_secretbox_KEYBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_secretbox_NONCEBYTES", (int)crypto_secretbox_NONCEBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_pwhash_SALTBYTES", (int)crypto_pwhash_SALTBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_pwhash_ALG_DEFAULT", (int)crypto_pwhash_ALG_DEFAULT);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_pwhash_OPSLIMIT_INTERACTIVE", (int)crypto_pwhash_OPSLIMIT_INTERACTIVE);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_pwhash_MEMLIMIT_INTERACTIVE", (int)crypto_pwhash_MEMLIMIT_INTERACTIVE);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_box_PUBLICKEYBYTES", (int)crypto_box_PUBLICKEYBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_box_SECRETKEYBYTES", (int)crypto_box_SECRETKEYBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_aead_xchacha20poly1305_ietf_KEYBYTES", (int)crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  jsiRuntime.global().setProperty(jsiRuntime, "crypto_kdf_KEYBYTES", (int)crypto_kdf_KEYBYTES);

  auto multiply = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forAscii(jsiRuntime, "multiply"), // internal function name
      1,                                                 // number of arguments
      // needs to return a jsi::Value
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        double res = 24;
        return jsi::Value(res);
      });

  // Registers the function on the global object
  jsiRuntime.global().setProperty(jsiRuntime, "multiply", std::move(multiply));

  auto from_base64_to_arraybuffer = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forUtf8(jsiRuntime, "from_base64"),
      2,
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        if (arguments[0].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][from_base64_to_arraybuffer] value can't be null");
        }
        if (arguments[1].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][from_base64_to_arraybuffer] variant can't be null");
        }

        std::string base64String = arguments[0].asString(runtime).utf8(runtime);
        uint8_t variant = arguments[1].asNumber();

        std::vector<uint8_t> uint8Vector;
        uint8Vector.resize(base64String.size());

        size_t length = 0;
        sodium_base642bin((uint8_t *)uint8Vector.data(), uint8Vector.size(), (char *)base64String.data(), base64String.size(), nullptr, &length, nullptr, variant);

        uint8Vector.resize(length);

        jsi::Object returnBufferAsObject = runtime.global()
                                               .getPropertyAsFunction(runtime, "ArrayBuffer")
                                               .callAsConstructor(runtime, (int)length)
                                               .asObject(runtime);

        jsi::ArrayBuffer arraybuffer = returnBufferAsObject.getArrayBuffer(runtime);
        memcpy(arraybuffer.data(runtime), uint8Vector.data(), uint8Vector.size());

        return returnBufferAsObject;
      });

  jsiRuntime.global().setProperty(jsiRuntime, "from_base64_to_arraybuffer", std::move(from_base64_to_arraybuffer));

  auto jsi_to_base64_from_string = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forUtf8(jsiRuntime, "jsi_to_base64_from_string"),
      2,
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        if (arguments[0].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][jsi_to_base64_from_string] value can't be null");
        }
        if (arguments[1].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][jsi_to_base64_from_string] variant can't be null");
        }

        std::string utf8String = arguments[0].asString(runtime).utf8(runtime);
        uint8_t variant = arguments[1].asNumber();

        std::string base64String;
        base64String.resize(sodium_base64_encoded_len(utf8String.size(), variant));
        sodium_bin2base64((char *)base64String.data(), base64String.size(), (uint8_t *)utf8String.data(), utf8String.size(), variant);

        return jsi::String::createFromUtf8(runtime, base64String);
      });
  jsiRuntime.global().setProperty(jsiRuntime, "jsi_to_base64_from_string", std::move(jsi_to_base64_from_string));

  auto jsi_to_base64_from_arraybuffer = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forUtf8(jsiRuntime, "jsi_to_base64_from_arraybuffer"),
      2,
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        if (arguments[0].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][jsi_to_base64_from_arraybuffer] value can't be null");
        }
        if (!arguments[0].isObject() ||
            !arguments[0].asObject(runtime).isArrayBuffer(runtime))
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][jsi_to_base64_from_arraybuffer] value must be an ArrayBuffer");
        }
        if (arguments[1].isNull())
        {
          throw jsi::JSError(runtime, "[react-native-rnlibsodium][jsi_to_base64_from_arraybuffer] variant can't be null");
        }

        auto dataArrayBuffer =
            arguments[0].asObject(runtime).getArrayBuffer(runtime);
        const unsigned char *data = dataArrayBuffer.data(runtime);
        auto dataLength = dataArrayBuffer.length(runtime);

        uint8_t variant = arguments[1].asNumber();

        std::string base64String;
        base64String.resize(sodium_base64_encoded_len(dataLength, variant));
        sodium_bin2base64((char *)base64String.data(), base64String.size(), data, dataLength, variant);

        return jsi::String::createFromUtf8(runtime, base64String);
      });

  jsiRuntime.global().setProperty(jsiRuntime, "jsi_to_base64_from_arraybuffer", std::move(jsi_to_base64_from_arraybuffer));

  auto jsi_crypto_secretbox_keygen = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forUtf8(jsiRuntime, "from_base64"),
      0,
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        unsigned char key[crypto_secretbox_KEYBYTES];
        crypto_secretbox_keygen(key);

        jsi::Object returnBufferAsObject = runtime.global()
                                               .getPropertyAsFunction(runtime, "ArrayBuffer")
                                               .callAsConstructor(runtime, (int)sizeof(key))
                                               .asObject(runtime);
        jsi::ArrayBuffer arraybuffer = returnBufferAsObject.getArrayBuffer(runtime);
        memcpy(arraybuffer.data(runtime), key, sizeof(key));
        return returnBufferAsObject;
      });

  jsiRuntime.global().setProperty(jsiRuntime, "jsi_crypto_secretbox_keygen", std::move(jsi_crypto_secretbox_keygen));

  auto jsi_crypto_aead_xchacha20poly1305_ietf_keygen = jsi::Function::createFromHostFunction(
      jsiRuntime,
      jsi::PropNameID::forUtf8(jsiRuntime, "from_base64"),
      0,
      [](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
      {
        unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
        crypto_aead_xchacha20poly1305_ietf_keygen(key);

        jsi::Object returnBufferAsObject = runtime.global()
                                               .getPropertyAsFunction(runtime, "ArrayBuffer")
                                               .callAsConstructor(runtime, (int)sizeof(key))
                                               .asObject(runtime);
        jsi::ArrayBuffer arraybuffer = returnBufferAsObject.getArrayBuffer(runtime);
        memcpy(arraybuffer.data(runtime), key, sizeof(key));
        return returnBufferAsObject;
      });

  jsiRuntime.global().setProperty(jsiRuntime, "jsi_crypto_aead_xchacha20poly1305_ietf_keygen", std::move(jsi_crypto_aead_xchacha20poly1305_ietf_keygen));
}

void cleanUpRnlibsodium()
{
  // intentionally left blank
}