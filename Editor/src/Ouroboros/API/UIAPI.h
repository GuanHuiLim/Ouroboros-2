#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/UI/UITextComponent.h"
#include "Ouroboros/UI/UIImageComponent.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* Text Functions for C#                                                       */
    /*-----------------------------------------------------------------------------*/

    SCRIPT_API ScriptDatabase::IntPtr UITextComponent_GetText(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::string const& name = obj->GetComponent<UITextComponent>().Text;
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API void UITextComponent_SetText(Scene::ID_type sceneID, oo::UUID uuid, const char* newText)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        obj->GetComponent<UITextComponent>().Text = newText;
    }

    SCRIPT_API_GET_SET(UITextComponent, Color, Color, TextColor)
    SCRIPT_API_GET_SET(UITextComponent, FontSize, float, FontSize)

    /*-----------------------------------------------------------------------------*/
    /* Image Functions for C#                                                      */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API AssetID UIImageComponent_GetSprite(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return obj->GetComponent<UIImageComponent>().GetAlbedoMap().GetID();
    }

    SCRIPT_API void UIImageComponent_SetSprite(Scene::ID_type sceneID, UUID uuid, AssetID assetID)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        if (assetID == 0)
        {
            obj->GetComponent<UIImageComponent>().SetAlbedoMap(Asset{});
            return;
        }
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL || asset.GetType() != AssetInfo::Type::Texture)
            ScriptEngine::ThrowNullException();
        obj->GetComponent<UIImageComponent>().SetAlbedoMap(asset);
    }

    SCRIPT_API_GET_SET(UIImageComponent, Color, Color, Tint)
}




