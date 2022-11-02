using System;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ouroboros
{
    /// <summary>
    /// Helper Functions used by the Ouroboros C++ Engine, should not be used
    /// </summary>
    namespace Engine
    {
        public static class Utility
        {
            private delegate void UtilityButtonAction(IntPtr ptr);

            public static ButtonAction GetButtonActionFromFunctionPointer(IntPtr fptr, IntPtr pptr)
            {
                UtilityButtonAction action = Marshal.GetDelegateForFunctionPointer<UtilityButtonAction>(fptr);
                return () =>
                {
                    action.Invoke(pptr);
                };
            }

            public static bool CheckMethodVoidReturnType(MethodInfo method)
            {
                return method.ReturnType == typeof(void);
            }

            public static bool CheckFieldStatic(object obj, string fieldName)
            {
                FieldInfo field = obj.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
                return field != null && field.IsStatic;
            }

            public static bool CheckFieldPublic(object obj, string fieldName)
            {
                FieldInfo field = obj.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.Public);
                return field != null && field.IsPublic;
            }

            public static string[] GetCustomFieldAttributes(object obj, string fieldName)
            {
                FieldInfo field = obj.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
                object[] attrList = field.GetCustomAttributes(false);
                string[] nameList = new string[attrList.Length];
                for (int i = 0; i < attrList.Length; ++i)
                {
                    nameList[i] = attrList[i].GetType().ToString();
                }
                return nameList;
            }

            public static bool HasCustomFieldAttribute(object obj, string fieldName, Type attribute)
            {
                FieldInfo field = obj.GetType().GetField(fieldName, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
                if (field == null)
                    return false;
                object[] attrList = field.GetCustomAttributes(attribute, false);
                return attrList.Length > 0;
            }

            public static bool HasCustomFieldAttribute(Type type, Type attribute)
            {
                return type.GetCustomAttribute(attribute) != null;
            }

            public static object CreateInstanceOfGenericList(Type genericType, Type arg)
            {
                Type[] args = new Type[1];
                args[0] = arg;
                Type fullType = genericType.MakeGenericType(args);
                return Activator.CreateInstance(fullType);
            }
        }
    }
}
