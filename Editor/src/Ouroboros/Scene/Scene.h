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

        std::string GetFilePath() const;
        std::string GetSceneName() const;

    protected:
        void SetFilePath(std::string_view filepath);
        void SetSceneName(std::string_view name);

        void LoadFromFile();
        void SaveToFile();

    private:
        std::string m_name;
        std::string m_filepath;
    };
}
