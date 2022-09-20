#include "ScriptEngine.h"

#include <fstream>
#include <assert.h>

namespace oo
{
    std::unordered_map<std::string, MonoImage*> ScriptEngine::libraryMap{};

    void ScriptEngine::Compile(std::string const& projPath, std::string const& warningsPath, std::string const& errorsPath)
    {
        if (IsLoaded())
            Unload();

        // find msbuild path
        FILE* msbuildPathFile = _popen("\"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe", "r");
        if (msbuildPathFile == nullptr)
            throw std::exception("ScriptEngine Compile Exception: Failed to find msbuild");

        char buffer[1024];
        fgets(buffer, 1024, msbuildPathFile);
        std::string msbuildPath(buffer);
        msbuildPath = msbuildPath.substr(0, msbuildPath.size() - 1);
        _pclose(msbuildPathFile);

        // execute build command
        std::string command("\"\"" + msbuildPath + "\" \"" + projPath + "\" -noLogo -verbosity:quiet -t:Build -fl1 -flp1:logfile=\"" 
                            + errorsPath + "\";errorsonly -fl2 -flp2:logfile=\"" + warningsPath 
                            + "\";warningsonly -p:Configuration=\"Debug OpenGL\";Platform=\"x64\"");
        FILE* compileResult = _popen(command.c_str(), "r");
        if (compileResult == nullptr)
            throw std::exception("ScriptEngine Compile Exception: msbuild failed to compile");
        _pclose(compileResult);
    }

    void ScriptEngine::Load(std::string const& dllPath)
    {
        if (IsLoaded())
            Unload();

        // load scripting dll
        if (mono_domain_get() == nullptr)
        {
            mono_set_dirs(".", "");
            mono_config_parse(nullptr);
            mono_debug_init(MONO_DEBUG_FORMAT_MONO);
            mono_jit_init("root");
            mono_thread_set_main(mono_thread_current());
            mono_assemblies_init();
        }
        MonoDomain* domain = mono_domain_create_appdomain((char*)"scripts", NULL);
        if (!mono_domain_set(domain, false))
            throw std::exception("ScriptEngine Load Exception: failed to set scripting domain");

        MonoAssembly* assembly = mono_domain_assembly_open(domain, dllPath.c_str());
        if (assembly == nullptr)
            throw std::exception("ScriptEngine Load Exception: failed to load assembly");

        MonoImage* scripting = mono_assembly_get_image(assembly);
        libraryMap.insert(std::pair<const char*, MonoImage*>{ "Scripting", mono_assembly_get_image(assembly) });

        // get ScriptCore Image
        MonoTableInfo const* tableInfo = mono_image_get_table_info(scripting, MONO_TABLE_ASSEMBLYREF);
        unsigned int tableRows = mono_table_info_get_rows(tableInfo);
        MonoAssemblyName* aName = mono_assembly_name_new("Temp");
        MonoImageOpenStatus status;
        for (unsigned int i = 0; i < tableRows; ++i)
        {
            mono_assembly_get_assemblyref(scripting, i, aName);
            MonoAssembly* refAssembly = mono_assembly_loaded(aName);
            if (refAssembly == nullptr)
            {
                std::string assemblyName = mono_assembly_name_get_name(aName);
                if (assemblyName == std::string{ "ScriptCore" })
                {
                    mono_assembly_load_reference(scripting, i);
                    refAssembly = mono_assembly_loaded(aName);
                    status = (refAssembly) ? MONO_IMAGE_OK : MONO_IMAGE_ERROR_ERRNO;
                    if (refAssembly != nullptr)
                        libraryMap.insert(std::pair<const char*, MonoImage*>{ assemblyName.c_str(), mono_assembly_get_image(refAssembly) });
                }
                else
                {
                    refAssembly = mono_assembly_load_full(aName, "mono/4.5", &status, false);
                    libraryMap.insert(std::pair<const char*, MonoImage*>{ assemblyName.c_str(), mono_assembly_get_image(refAssembly) });
                }
            }
        }
        // mono_assembly_name_free(aName);
    }

    bool ScriptEngine::IsLoaded()
    {
        return libraryMap.size() > 0;
    }

    void ScriptEngine::Unload()
    {
        // unload current app domain
        MonoDomain* oldDomain = mono_domain_get();
        if (oldDomain != nullptr && oldDomain != mono_get_root_domain())
        {
            if (!mono_domain_set(mono_get_root_domain(), false))
                throw std::exception("ScriptEngine Unload Exception: failed to set root domain");
            mono_domain_unload(oldDomain);
            // Trigger C# garbage collection, not necessary but good point to clean up stuff
            mono_gc_collect(mono_gc_max_generation());
        }
        libraryMap.clear();
    }

    void ScriptEngine::Shutdown()
    {
        Unload();
        if (mono_domain_get() != nullptr)
            mono_jit_cleanup(mono_domain_get());
    }

    // Creators

    MonoObject* ScriptEngine::CreateObject(MonoClass* klass)
    {
        return mono_object_new(mono_domain_get(), klass);
    }

    MonoString* ScriptEngine::CreateString(const char* text)
    {
        return mono_string_new(mono_domain_get(), text);
    }

    // Accessors

    MonoImage* ScriptEngine::GetLibrary(const char* aLibrary)
    {
        auto search = libraryMap.find(aLibrary);
        if (search == libraryMap.end())
            return nullptr;
        return search->second;
    }

    MonoClass* ScriptEngine::GetClass(const char* aLibrary, const char* aNamespace, const char* aClassName)
    {
        MonoImage* library = GetLibrary(aLibrary);
        assert(library != nullptr);
        //if (library == nullptr)
            //throw std::exception("ScriptEngine GetClass Exception: the Library doesn't exist");
        MonoClass* klass = mono_class_from_name(library, aNamespace, aClassName);
        assert(klass != nullptr);
        //if (klass == nullptr)
            //throw std::exception("ScriptEngine GetClass Exception: class doesn't exist");
        return klass;
    }

    MonoClass* ScriptEngine::TryGetClass(const char* aLibrary, const char* aNamespace, const char* aClassName)
    {
        MonoImage* library = GetLibrary(aLibrary);
        if (library == nullptr)
            return nullptr;
        MonoClass* klass = mono_class_from_name(library, aNamespace, aClassName);
        if (klass == nullptr)
            return nullptr;
        return klass;
    }

    std::vector<MonoClass*> const ScriptEngine::GetClassesByBaseClass(const char* aLibrary, MonoClass* baseClass)
    {
        std::vector<MonoClass*> classList{};
        if (baseClass == nullptr)
            return classList;
        MonoImage* library = GetLibrary(aLibrary);
        if (library == nullptr)
            return classList;

        MonoTableInfo const* tableInfo = mono_image_get_table_info(library, MONO_TABLE_TYPEDEF);
        unsigned tableRows = mono_table_info_get_rows(tableInfo);
        for (unsigned int i = 1; i < tableRows; ++i)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(tableInfo, i, cols, MONO_TYPEDEF_SIZE);
            const char* name = mono_metadata_string_heap(library, cols[MONO_TYPEDEF_NAME]);
            const char* name_space = mono_metadata_string_heap(library, cols[MONO_TYPEDEF_NAMESPACE]);
            MonoClass* klass = mono_class_from_name(library, name_space, name);

            if (klass != baseClass && CheckClassInheritance(klass, baseClass))
                classList.emplace_back(klass);
        }
        return classList;
    }

    std::string const ScriptEngine::GetClassInfoNameSpace(MonoClass* klass)
    {
        return mono_class_get_namespace(klass);
    }
    std::string const ScriptEngine::GetClassInfoName(MonoClass* klass)
    {
        std::string name = mono_class_get_name(klass);
        MonoClass* nestingKlass = mono_class_get_nesting_type(klass);
        while (nestingKlass != nullptr)
        {
            name = std::string{ mono_class_get_name(nestingKlass) } + "/" + name;
            nestingKlass = mono_class_get_nesting_type(nestingKlass);
        }
        return name;
    }

    MonoMethod* ScriptEngine::GetFunction(MonoClass* klass, const char* functionName, int paramCount)
    {
        MonoMethod* method = mono_class_get_method_from_name(klass, functionName, paramCount);
        MonoClass* pClass = mono_class_get_parent(klass);

        while (method == nullptr && pClass != nullptr && klass != pClass)
        {
            klass = pClass;
            pClass = mono_class_get_parent(klass);
            method = mono_class_get_method_from_name(klass, functionName, paramCount);
        }
        return method;
    }

    MonoMethod* ScriptEngine::GetFunction(MonoObject* obj, const char* functionName, int paramCount)
    {
        return GetFunction(mono_object_get_class(obj), functionName, paramCount);
    }

    std::vector<std::string> const ScriptEngine::GetClassFieldAttributes(MonoObject* obj, const char* fieldName)
    {
        MonoString* fieldNameString = CreateString(fieldName);

        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "GetCustomFieldAttributes", 2);
        void* params[2];
        params[0] = obj;
        params[1] = fieldNameString;
        MonoArray* results = (MonoArray*)mono_runtime_invoke(utilMethod, NULL, params, NULL);

        uintptr_t arrayLength = mono_array_length(results);
        std::vector<std::string> attrList;
        for (size_t i = 0; i < arrayLength; ++i)
        {
            MonoString* typeString = mono_array_get(results, MonoString*, i);
            attrList.emplace_back(mono_string_to_utf8(typeString));
        }
        return attrList;
    }

    std::vector<MonoType*> ScriptEngine::GetTypeGenericTypes(MonoType* type)
    {
        std::vector<MonoType*> resultList;
        MonoObject* typeObj = (MonoObject*)mono_type_get_object(mono_domain_get(), type);
        MonoClass* typeClass = mono_object_get_class(typeObj);
        MonoMethod* getMethod = mono_class_get_method_from_name(typeClass, "GetGenericArguments", 0);
        MonoArray* resultArray = (MonoArray*)mono_runtime_invoke(getMethod, typeObj, NULL, NULL);
        uintptr_t arrayLength = mono_array_length(resultArray);
        for (unsigned int i = 0; i < arrayLength; ++i)
        {
            MonoReflectionType* genericType = mono_array_get(resultArray, MonoReflectionType*, i);
            resultList.emplace_back(mono_reflection_type_get_type(genericType));
        }
        return resultList;
    }

    std::vector<std::string> ScriptEngine::GetEnumOptions(MonoType* enumType)
    {
        MonoMethod* GetEnumNames = mono_class_get_method_from_name(mono_get_enum_class(), "GetNames", 1);
        void* args[1];
        args[0] = mono_type_get_object(mono_domain_get(), enumType);
        MonoArray* enumNames = (MonoArray*)mono_runtime_invoke(GetEnumNames, NULL, args, NULL);

        unsigned int totalCount = (unsigned int)mono_array_length(enumNames);
        std::vector<std::string> optionList(totalCount);
        for (unsigned int i = 0; i < totalCount; ++i)
        {
            optionList[i] = mono_string_to_utf8(mono_array_get(enumNames, MonoString*, i));
        }
        return optionList;
    }

    // Checks
    bool ScriptEngine::CheckClassExists(const char* aLibrary, const char* aNamespace, const char* aClassName)
    {
        MonoImage* library = GetLibrary(aLibrary);
        if (library == nullptr)
            return false;
        return mono_class_from_name(library, aNamespace, aClassName) != nullptr;
    }

    bool ScriptEngine::CheckClassInheritance(MonoClass* derivedClass, MonoClass* baseClass)
    {
        while (derivedClass != nullptr)
        {
            if (derivedClass == baseClass)
                return true;
            derivedClass = mono_class_get_parent(derivedClass);
        }
        return false;
    }

    bool ScriptEngine::CheckClassInheritance(MonoClass* derivedClass, const char* aLibrary, const char* aNamespace, const char* aClassName)
    {
        MonoClass* baseClass = GetClass(aLibrary, aNamespace, aClassName);
        return CheckClassInheritance(derivedClass, baseClass);
    }

    bool ScriptEngine::CheckClassFieldStatic(MonoObject* obj, const char* fieldName)
    {
        MonoString* fieldNameString = CreateString(fieldName);

        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "CheckFieldStatic", 2);
        void* params[2];
        params[0] = obj;
        params[1] = fieldNameString;
        MonoObject* result = mono_runtime_invoke(utilMethod, NULL, params, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassFieldPublic(MonoObject* obj, const char* fieldName)
    {
        MonoString* fieldNameString = CreateString(fieldName);

        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "CheckFieldPublic", 2);
        void* params[2];
        params[0] = obj;
        params[1] = fieldNameString;
        MonoObject* result = mono_runtime_invoke(utilMethod, NULL, params, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassFieldHasAttribute(MonoObject* obj, const char* fieldName, MonoType* attributeType)
    {
        MonoString* fieldNameString = CreateString(fieldName);
        MonoReflectionType* attrRefType = mono_type_get_object(mono_domain_get(), attributeType);

        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "HasCustomFieldAttribute", 3);
        void* params[3];
        params[0] = obj;
        params[1] = fieldNameString;
        params[2] = attrRefType;
        MonoObject* result = mono_runtime_invoke(utilMethod, NULL, params, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassFieldInspectorVisible(MonoObject* obj, MonoClassField* field)
    {
        const char* fieldName = mono_field_get_name(field);

        if (ScriptEngine::CheckClassFieldStatic(obj, fieldName))
            return false;

        if (ScriptEngine::CheckClassFieldPublic(obj, fieldName))
        {
            MonoType* attrType = mono_class_get_type(ScriptEngine::GetClass("ScriptCore", "Ouroboros", "NonSerialized"));
            return !ScriptEngine::CheckClassFieldHasAttribute(obj, fieldName, attrType);
        }
        else
        {
            MonoType* attrType = mono_class_get_type(ScriptEngine::GetClass("ScriptCore", "Ouroboros", "SerializeField"));
            return ScriptEngine::CheckClassFieldHasAttribute(obj, fieldName, attrType);
        }
    }

    bool ScriptEngine::CheckTypeHasAttribute(MonoType* type, MonoType* attribute)
    {
        MonoReflectionType* typeRefType = mono_type_get_object(mono_domain_get(), type);
        MonoReflectionType* attrRefType = mono_type_get_object(mono_domain_get(), attribute);

        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "HasCustomFieldAttribute", 2);
        void* params[2];
        params[0] = typeRefType;
        params[1] = attrRefType;
        MonoObject* result = mono_runtime_invoke(utilMethod, NULL, params, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassMethodStatic(MonoClass* klass, MonoMethod* method)
    {
        MonoObject* methodObject = reinterpret_cast<MonoObject*>(mono_method_get_object(mono_domain_get(), method, klass));
        MonoProperty* staticProperty = mono_class_get_property_from_name(mono_object_get_class(methodObject), "IsStatic");
        MonoObject* result = mono_property_get_value(staticProperty, methodObject, NULL, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassMethodPublic(MonoClass* klass, MonoMethod* method)
    {
        MonoObject* methodObject = reinterpret_cast<MonoObject*>(mono_method_get_object(mono_domain_get(), method, klass));
        MonoProperty* publicProperty = mono_class_get_property_from_name(mono_object_get_class(methodObject), "IsPublic");
        MonoObject* result = mono_property_get_value(publicProperty, methodObject, NULL, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckClassMethodReturnVoid(MonoClass* klass, MonoMethod* method)
    {
        MonoObject* methodObject = reinterpret_cast<MonoObject*>(mono_method_get_object(mono_domain_get(), method, klass));
        MonoClass* utilClass = GetClass("ScriptCore", "Ouroboros.Engine", "Utility");
        MonoMethod* utilMethod = mono_class_get_method_from_name(utilClass, "CheckMethodVoidReturnType", 1);
        void* params[1];
        params[0] = methodObject;
        MonoObject* result = mono_runtime_invoke(utilMethod, NULL, params, NULL);
        return *(bool*)(mono_object_unbox(result));
    }

    bool ScriptEngine::CheckGenericList(MonoType* type)
    {
        MonoObject* typeObj = (MonoObject*)mono_type_get_object(mono_domain_get(), type);
        MonoClass* typeClass = mono_object_get_class(typeObj);

        MonoProperty* genericProperty = mono_class_get_property_from_name(typeClass, "IsGenericType");
        bool isGenericType = *(bool*)mono_object_unbox(mono_property_get_value(genericProperty, typeObj, NULL, NULL)); //*(bool*)mono_property_get_value(genericProperty, typeObj, NULL, NULL);
        if (!isGenericType)
            return false;

        MonoClass* listClass = mono_class_from_name(mono_get_corlib(), "System.Collections.Generic", "List`1");
        MonoMethod* defMethod = mono_class_get_method_from_name(typeClass, "GetGenericTypeDefinition", 0);
        MonoReflectionType* result = (MonoReflectionType*)mono_runtime_invoke(defMethod, typeObj, NULL, NULL);
        MonoClass* resultClass = mono_type_get_class(mono_reflection_type_get_type(result));
        return listClass == resultClass;
    }

    // Actions

    MonoObject* ScriptEngine::InvokeFunction(MonoObject* obj, MonoMethod* method, void** paramList)
    {
        if (method == nullptr)
            return nullptr;
        MonoObject* exception = nullptr;
        MonoObject* result = mono_runtime_invoke(method, obj, paramList, &exception);
        if (exception)
        {
            MonoMethod* stringMethod = mono_class_get_method_from_name(mono_get_exception_class(), "ToString", 0);
            MonoString* excString = (MonoString*)mono_runtime_invoke(stringMethod, exception, NULL, NULL);
            throw std::exception(mono_string_to_utf8(excString));
        }
        return result;
    }

    MonoObject* ScriptEngine::InvokeFunction(MonoObject* obj, const char* functionName, void** paramList, int paramCount)
    {
        return InvokeFunction(obj, GetFunction(obj, functionName, paramCount), paramList);
    }

    void ScriptEngine::ThrowNullException()
    {
        mono_raise_exception(mono_get_exception_null_reference());
    }

    void ScriptEngine::ThrowOutOfIndexException()
    {
        mono_raise_exception(mono_get_exception_index_out_of_range());
    }
}