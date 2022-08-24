#ifndef EvaValue_h
#define EvaValue_h

/**
 * @brief
 * Eval type
 */
enum class EvaValueType {
    NUMBER,
    OBJECT,
};

/**
 * @brief
 * Object type
 */
enum class ObjectType {
    STRING,
    CODE,
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

/**
 * @brief
 * Eva value (tagged union)
 *
 */
struct EvaValue {
    EvaValueType type;
    union {
        double number;
        Object* object;
    };
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
};

// marcos:

//------------------------------
// constructors marcos, the brackest cast the value
#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})

#define ALLOC_STRING(value) ((EvaValue){EvaValueType::OBJECT, .object = (Object*)new StringObject(value)})
#define ALLOC_CODE(name) ((EvaValue){EvaValueType::OBJECT, .object = (Object*)new CodeObject(name)})
//------------------------------
// constructors
#define AS_NUMBER(evaValue) ((double)(evaValue).number)         // return the number
#define AS_STRING(evaValue) ((StringObject*)(evaValue).object)  // return the object
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)    // return the string
#define AS_OBJECT(evaValue) ((Object*)(evaValue).object)        // return object like AS_string
#define AS_CODE(evaValue) ((CodeObject*)(evaValue).object)      // return code
//-------------------------------
// Testers:

#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)
#define IS_OBJECT_TYPE(evaValue, objectType) \
    (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)
#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)

#endif