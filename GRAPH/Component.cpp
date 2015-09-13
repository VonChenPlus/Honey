#include "GRAPH/Component.h"
#include "GRAPH/Node.h"

namespace GRAPH
{
    Component::Component(void)
        : _owner(nullptr)
        , _enabled(true) {
    }

    Component::~Component(void) {
    }

    bool Component::init() {
        return true;
    }

    void Component::onEnter() {
    }

    void Component::onExit() {
    }

    void Component::onAdd() {
    }

    void Component::onRemove() {
    }

    void Component::update(float) {
    }

    bool Component::serialize(void *) {
        return true;
    }

    Component* Component::create(void) {
        Component * ret = new (std::nothrow) Component();
        if (ret != nullptr && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    const std::string& Component::getName() const {
        return _name;
    }

    void Component::setName(const std::string& name) {
        _name = name;
    }

    HObject* Component::getOwner() const {
        return _owner;
    }

    void Component::setOwner(HObject *owner) {
        _owner = owner;
    }

    bool Component::isEnabled() const {
        return _enabled;
    }

    void Component::setEnabled(bool enable) {
        _enabled = enable;
    }

    ComponentContainer::ComponentContainer(HObject *object)
        : _components(nullptr)
        , _owner(object) {
    }

    ComponentContainer::~ComponentContainer(void) {
        SAFE_DELETE(_components);
    }

    Component* ComponentContainer::get(const std::string& name) const {
        Component* ret = nullptr;
        do {
            if(nullptr == _components)break;
            ret = _components->at(name);
        } while (0);
        return ret;
    }

    bool ComponentContainer::add(Component *com) {
        bool ret = false;
        do {
            if (_components == nullptr) {
                _components = new (std::nothrow) HObjectMap<std::string, Component*>();
            }

            Component *component = _components->at(com->getName());
            if(component) break;
            com->setOwner(_owner);
            _components->insert(com->getName(), com);
            com->onAdd();
            ret = true;
        } while(0);
        return ret;
    }

    bool ComponentContainer::remove(const std::string& name) {
        bool ret = false;
        do {
            if(!_components) break;

            auto iter = _components->find(name);
            if(iter == _components->end()) break;

            auto com = iter->second;
            com->onRemove();
            com->setOwner(nullptr);

            _components->erase(iter);
            ret = true;
        } while(0);
        return ret;
     }

    bool ComponentContainer::remove(Component *com) {
        bool ret = false;
        do {
            if(!_components) break;

            for (auto iter = _components->begin(); iter != _components->end(); ++iter) {
                if (iter->second == com) {
                    com->onRemove();
                    com->setOwner(nullptr);
                    _components->erase(iter);
                    break;
                }
            }
            ret = true;
        } while(0);

        return ret;
    }

    void ComponentContainer::removeAll() {
        if (_components != nullptr) {
            for (auto iter = _components->begin(); iter != _components->end(); ++iter) {
                iter->second->onRemove();
                iter->second->setOwner(nullptr);
            }

            _components->clear();
            SAFE_DELETE(_components);

            dynamic_cast<Node *>(_owner)->unscheduleUpdate();
        }
    }

    void ComponentContainer::alloc(void) {
        _components = new (std::nothrow) HObjectMap<std::string, Component*>();
    }

    void ComponentContainer::visit(float delta) {
        if (_components != nullptr) {
            SAFE_RETAIN(_owner);
            for (auto iter = _components->begin(); iter != _components->end(); ++iter) {
                iter->second->update(delta);
            }
            SAFE_RELEASE(_owner);
        }
    }

    bool ComponentContainer::isEmpty() const {
        return (_components == nullptr || _components->empty());
    }

    void ComponentContainer::onEnter() {
        for (auto iter = _components->begin(); iter != _components->end(); ++iter) {
            iter->second->onEnter();
        }
    }

    void ComponentContainer::onExit() {
        for (auto iter = _components->begin(); iter != _components->end(); ++iter) {
            iter->second->onExit();
        }
    }
}
