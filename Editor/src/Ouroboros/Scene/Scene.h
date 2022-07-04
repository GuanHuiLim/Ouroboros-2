#pragma once

#include <IScene.h>
#include "Ouroboros/EventSystem/Event.h"
namespace oo
{
    class Scene : public IScene
    {
        // Events
    public:
        class OnInitEvent : public Event
        {
        };

    public:
        Scene(std::string_view name) : m_name{ name }, IScene() {};
        virtual ~Scene() = default;

        virtual void Init() override;
        virtual void Update() override;
        virtual void LateUpdate() override;
        virtual void Render() override;
        virtual void EndOfFrameUpdate() override;
        virtual void Exit() override;

        virtual void LoadScene() override;
        virtual void UnloadScene() override;
        virtual void ReloadScene() override;

        virtual LoadStatus GetProgress() override;

    protected:
        void SetSaveFile(std::string_view filepath);
        std::string GetFilePath() const;

        void SetSceneName(std::string_view name);
        std::string GetSceneName() const;

        void LoadFromFile();
        void SaveToFile();

    private:
        std::string m_name;
        std::string m_filepath;
    };
}
