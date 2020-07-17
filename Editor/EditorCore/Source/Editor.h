#pragma once
#include <pch.h>
#include <Renderer/Module.h>

namespace Squid {
namespace EditorCore {

    class Dockspace;
    class LogWidget;
    class SceneWidget;
    class ViewportWidget;
    class PropertiesWidget;

    class Editor {
    public:
        Editor(std::shared_ptr<Renderer::Module> renderer);
        ~Editor();

        void EditorBegin();
        void EditorEnd();
        void Tick();

    private:
        void SetupStyles() const;
    
        std::unique_ptr<Dockspace> dockspace;
        std::unique_ptr<SceneWidget> scene_widget;
        std::unique_ptr<ViewportWidget> viewport_widget;
        std::unique_ptr<LogWidget> log_widget;
        std::unique_ptr<PropertiesWidget> properties_widget;

        std::shared_ptr<Renderer::Module> renderer;
    };

} // namespace EditorCore
} // namespace Squid
