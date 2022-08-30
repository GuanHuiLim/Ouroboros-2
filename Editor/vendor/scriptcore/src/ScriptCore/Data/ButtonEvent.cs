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
            foreach(ButtonAction action in actionList)
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