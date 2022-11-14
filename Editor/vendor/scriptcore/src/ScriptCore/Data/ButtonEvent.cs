using System.Collections.Generic;

namespace Ouroboros
{
    public delegate void ButtonAction();

    public class ButtonEvent
    {
        //public delegate void ButtonAction();

        private List<ButtonAction> actionList = new List<ButtonAction>();

        public void Invoke()
        {
            List<ButtonAction> copy = new List<ButtonAction>(actionList);
            foreach(ButtonAction action in copy)
            {
                action.Invoke();
            }
        }

        public void AddListener(ButtonAction action)
        {
            actionList.Add(action);
        }

        public void RemoveListener(ButtonAction action)
        {
            actionList.Remove(action);
        }

        public void RemoveAllListeners()
        {
            actionList.Clear();
        }
    }
}