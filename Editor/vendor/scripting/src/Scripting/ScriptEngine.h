#pragma once

#include <string>
#include <unordered_map>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/exception.h>

namespace oo
{
    class ScriptEngine
    {
    public:
        // Exceptions
        class CompileException : public std::exception
        {
        public:
            enum class Type
            {
                MSBUILD_NOTFOUND,
                MSBUILD_FAILED,
            };

        public:
            CompileException(Type aType, const char* aMessage) : type{ aType }, message{ aMessage } {};

        public:
            Type type;
            const char* message;
        };

        class LoadException : public std::exception
        {
        public:
            enum class Type
            {
                DOMAIN_FAILED,
                ASSEMBLY_FAILED,
            };

        public:
            LoadException(Type aType, const char* aMessage) : type{ aType }, message{ aMessage } {};

        public:
            Type type;
            const char* message;
        };

        class ScriptException : public std::exception
        {
        public:
            ScriptException(const char* aMessage) : message{ aMessage } {}

        public:
            const char* message;
        };

    public:
        // Control functions
        static void Compile(std::string const& projPath, std::string const& warningsPath, std::string const& errorsPath);
        static void Load(std::string const& dllPath);
        static void Unload();
        static bool IsLoaded();
        static void Shutdown();

        // Creators
        static MonoObject* CreateObject(MonoClass* klass);
        static MonoString* CreateString(const char* text);

        // Accessors
        static MonoImage* GetLibrary(const char* aLibrary);
        static MonoClass* GetClass(const char* aLibrary, const char* aNamespace, const char* aClassName);
        static std::vector<MonoClass*> const GetClassesByBaseClass(const char* aLibrary, MonoClass* baseClass);
        static MonoMethod* GetFunction(MonoClass* klass, const char* functionName, int paramCount = 0);
        static MonoMethod* GetFunction(MonoObject* obj, const char* functionName, int paramCount = 0);

        static std::vector<std::string> const GetClassFieldAttributes(MonoObject* obj, const char* fieldName);

        static std::vector<MonoType*> GetTypeGenericTypes(MonoType* type);
        static std::vector<std::string> GetEnumOptions(MonoType* type);

        // Checks
        static bool CheckClassInheritance(MonoClass* derivedClass, MonoClass* baseClass);
        static bool CheckClassInheritance(MonoClass* derivedClass, const char* aLibrary, const char* aNamespace, const char* aClassName);

        static bool CheckClassFieldStatic(MonoObject* obj, const char* fieldName);
        static bool CheckClassFieldPublic(MonoObject* obj, const char* fieldName);
        static bool CheckClassFieldHasAttribute(MonoObject* obj, const char* fieldName, MonoType* attributeType);
        static bool CheckClassFieldInspectorVisible(MonoObject* obj, MonoClassField* field);

        static bool CheckTypeHasAttribute(MonoType* type, MonoType* attribute);

        static bool CheckClassMethodStatic(MonoClass* klass, MonoMethod* method);
        static bool CheckClassMethodPublic(MonoClass* klass, MonoMethod* method);
        static bool CheckClassMethodReturnVoid(MonoClass* klass, MonoMethod* method);

        static bool CheckGenericList(MonoType* type);

        // Actions
        static MonoObject* InvokeFunction(MonoObject* obj, MonoMethod* method, void** paramList = nullptr);
        static MonoObject* InvokeFunction(MonoObject* obj, const char* functionName, void** paramList = nullptr,  int paramCount = 0);

        // Throw Exceptions
        static void ThrowNullException();

    private:
        static std::unordered_map<std::string, MonoImage*> libraryMap;
    };
}