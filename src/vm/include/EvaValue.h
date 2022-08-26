#ifndef EvaValue_h
#define EvaValue_h

#include <functional>
#include <string>
#include <vector>

#include "../../logger/include/Logger.h"
/**
 * @brief
 * Eval type
 */
enum class EvaValueType {
    NUMBER,
    BOOLEAN,
    OBJECT,
};

/**
 * @brief
 * Object type
 */
enum class ObjectType {
    STRING,
    CODE,
    NATIVE,
};

/**
 * @brief
 * Base object
 */

struct Object {
    Object(ObjectType type)
        : type(type) {}
    ObjectType type;
};

/**
 * @brief
 * String object
 */

struct StringObject : public Object {
    StringObject(const std::string& str)
        : Object(ObjectType::STRING), string(str) {}
    std::string string;
};

using NativeFn = std::function<void()>;

struct NativeObject : public Object {
    NativeObject(NativeFn function,
                 const std::string& name,
                 size_t arity)
        : Object(ObjectType::NATIVE),
          function(function),
          name(name),
          arity(arity) {}

    NativeFn function;

    std::string name;

    size_t arity;
};

/**
 * @brief
 * Eva value (tagged union)
 *
 */
struct EvaValue {
    EvaValueType type;
    union {
        double number;
        bool boolean;
        Object* object;
    };
};

struct LocalVar {
    std::string name;
    size_t scopeLevel;
};

/**
 * @brief
 * String object
 */

struct CodeObject : public Object {
    CodeObject(const std::string& name)
        : Object(ObjectType::CODE), name(name) {}
    std::string name;
    std::vector<EvaValue> constants;
    std::vector<uint8_t> code;

    size_t scopeLevel = 0;
    std::vector<LocalVar> locals;

    inline int getLocalIndex(const std::string& name) {
        if (locals.size() > 0) {
            // have to go from behind to get the newest stack value
            for (auto itr = locals.rbegin(); itr != locals.rend(); ++itr) {
                if (itr->name == name) {
                    return std::distance(itr, locals.rend()) - 1;
                }
            }
        }

        return -1;
    };

    inline void addLocal(const std::string& name) {
        locals.push_back({name, scopeLevel});
    };
};

// marcos:

//------------------------------
// constructors marcos, the brackest cast the value
#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})
#define BOOLEAN(value) ((EvaValue){EvaValueType::BOOLEAN, .boolean = value})

// confused name (change it)
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)  // return the string

#define ALLOC_STRING(value) ((EvaValue){EvaValueType::OBJECT, .object = (Object*)new StringObject(value)})
#define ALLOC_CODE(name) ((EvaValue){EvaValueType::OBJECT, .object = (Object*)new CodeObject(name)})
#define ALLOC_NATIVE(fn, name, arity) \
    ((EvaValue){EvaValueType::OBJECT, \
                .object = (Object*)new NativeObject(fn, name, arity)})
//------------------------------
// converter
#define AS_NUMBER(evaValue) ((double)(evaValue).number)  // return the number
#define AS_BOOLEAN(evaValue) ((bool)(evaValue).boolean)
#define AS_STRING(evaValue) ((StringObject*)(evaValue).object)  // return the object
#define AS_OBJECT(evaValue) ((Object*)(evaValue).object)        // return object like AS_string
#define AS_CODE(evaValue) ((CodeObject*)(evaValue).object)      // return code
#define AS_NATIVE(fnValue) ((NativeObject*)(fnValue).object)

//-------------------------------
// Testers:
#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_BOOLEAN(evaValue) ((evaValue).type == EvaValueType::BOOLEAN)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)
#define IS_OBJECT_TYPE(evaValue, objectType) \
    (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)
#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)
#define IS_NATIVE(fnValue) IS_OBJECT_TYPE(fnValue, ObjectType::NATIVE)

inline std::string evaValueToTypeString(const EvaValue& evaValue) {
    if (IS_NUMBER(evaValue)) {
        return "NUMBER";
    } else if (IS_BOOLEAN(evaValue)) {
        return "BOOLEAN";
    } else if (IS_STRING(evaValue)) {
        return "STRING";
    } else if (IS_CODE(evaValue)) {
        return "CODE";
    } else if (IS_NATIVE(evaValue)) {
        return "CODE";
    } else {
        DIE << "evaValueToTypeString: Unknown type" << (int)evaValue.type;
    }
    return "";
}

inline std::string evaValueToConstantString(const EvaValue& evaValue) {
    std::stringstream ss;
    if (IS_NUMBER(evaValue)) {
        ss << evaValue.number;
    } else if (IS_BOOLEAN(evaValue)) {
        ss << (evaValue.boolean == true ? "true" : "false");
    } else if (IS_STRING(evaValue)) {
        ss << '"' << AS_CPPSTRING(evaValue) << '"';
    } else if (IS_CODE(evaValue)) {
        auto code = AS_CODE(evaValue);
        ss << "code " << code << ": " << code->name;
    } else if (IS_NATIVE(evaValue)) {
        auto fn = AS_NATIVE(evaValue);
        ss << fn->name << "/" << fn->arity;
    } else {
        DIE << "evaValueToConstantString: unknown type " << (int)evaValue.type;
    }

    return ss.str();
}

inline std::ostream& operator<<(std::ostream& os, const EvaValue& evaValue) {
    return os << "EvaValue(" << evaValueToTypeString(evaValue) << "): " << evaValueToConstantString(evaValue);
}

#endif