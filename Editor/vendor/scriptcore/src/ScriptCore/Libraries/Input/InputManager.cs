using System.Runtime.InteropServices;

namespace Ouroboros
{
    public static class InputManager
    {
        public class Axis
        {
            private string name;

            public Axis(string _name)
            {
                this.name = _name;
            }

            public float GetValue()
            {
                return Input.GetAxis(name);
            }

            [DllImport("__Internal")] private static extern void InputManager_SetPositiveKeyCode(string axisName, KeyCode keyCode);
            [DllImport("__Internal")] private static extern void InputManager_SetPositiveMouseCode(string axisName, MouseCode mouseCode);
            [DllImport("__Internal")] private static extern void InputManager_SetPositiveControllerButtonCode(string axisName, ControllerButtonCode buttonCode);
            [DllImport("__Internal")] private static extern void InputManager_SetPositiveControllerAxisCode(string axisName, ControllerAxisCode axisCode);

            public void SetPositive(KeyCode key)
            {
                InputManager_SetPositiveKeyCode(name, key);
            }
            public void SetPositive(MouseCode mouse)
            {
                InputManager_SetPositiveMouseCode(name, mouse);
            }
            public void SetPositive(ControllerButtonCode button)
            {
                InputManager_SetPositiveControllerButtonCode(name, button);
            }
            public void SetPositive(ControllerAxisCode axis)
            {
                InputManager_SetPositiveControllerAxisCode(name, axis);
            }

            [DllImport("__Internal")] private static extern void InputManager_SetNegativeKeyCode(string axisName, KeyCode keyCode);
            [DllImport("__Internal")] private static extern void InputManager_SetNegativeMouseCode(string axisName, MouseCode mouseCode);
            [DllImport("__Internal")] private static extern void InputManager_SetNegativeControllerButtonCode(string axisName, ControllerButtonCode buttonCode);

            public void SetNegative(KeyCode key)
            {
                InputManager_SetNegativeKeyCode(name, key);
            }
            public void SetNegative(MouseCode mouse)
            {
                InputManager_SetNegativeMouseCode(name, mouse);
            }
            public void SetNegative(ControllerButtonCode button)
            {
                InputManager_SetNegativeControllerButtonCode(name, button);
            }

            [DllImport("__Internal")] private static extern bool InputManager_IsPositiveKeyCode(string axisName, KeyCode keyCode);
            [DllImport("__Internal")] private static extern bool InputManager_IsPositiveMouseCode(string axisName, MouseCode mouseCode);
            [DllImport("__Internal")] private static extern bool InputManager_IsPositiveControllerButtonCode(string axisName, ControllerButtonCode buttonCode);
            [DllImport("__Internal")] private static extern bool InputManager_IsPositiveControllerAxisCode(string axisName, ControllerAxisCode axisCode);

            public bool IsPositive(KeyCode key)
            {
                return InputManager_IsPositiveKeyCode(name, key);
            }
            public bool IsPositive(MouseCode key)
            {
                return InputManager_IsPositiveMouseCode(name, key);
            }
            public bool IsPositive(ControllerButtonCode key)
            {
                return InputManager_IsPositiveControllerButtonCode(name, key);
            }
            public bool IsPositive(ControllerAxisCode key)
            {
                return InputManager_IsPositiveControllerAxisCode(name, key);
            }

            [DllImport("__Internal")] private static extern bool InputManager_IsNegativeKeyCode(string axisName, KeyCode keyCode);
            [DllImport("__Internal")] private static extern bool InputManager_IsNegativeMouseCode(string axisName, MouseCode mouseCode);
            [DllImport("__Internal")] private static extern bool InputManager_IsNegativeControllerButtonCode(string axisName, ControllerButtonCode buttonCode);

            public bool IsNegative(KeyCode key)
            {
                return InputManager_IsNegativeKeyCode(name, key);
            }
            public bool IsNegative(MouseCode key)
            {
                return InputManager_IsNegativeMouseCode(name, key);
            }
            public bool IsNegative(ControllerButtonCode key)
            {
                return InputManager_IsNegativeControllerButtonCode(name, key);
            }
        }

        public static Axis GetAxis(string name)
        {
            return new Axis(name);
        }

        [DllImport("__Internal")] public static extern Axis[] GetAxesAll();

        [DllImport("__Internal")] private static extern void Project_SaveInputs(string filePath);

        public static void Save()
        {
            Project_SaveInputs("Custom_InputBindings");
        }

        [DllImport("__Internal")] private static extern bool Project_HasInputsFile(string filePath);
        [DllImport("__Internal")] private static extern void Project_LoadInputs(string filePath);

        public static void Revert()
        {
            if (Project_HasInputsFile("Custom_InputBindings"))
            {
                Project_LoadInputs("Custom_InputBindings");
            }
            else
            {
                Project_LoadInputs("InputBindings");
            }
        }

        public static void Reset()
        {
            Project_LoadInputs("InputBindings");
        }
    }
}