#include "GRAPH/Component.h"
#include "GRAPH/Node.h"

namespace GRAPH
{
    Component::Component(void)
        : owner_(nullptr)
        , enabled_(true) {
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
        return name_;
    }

    void Component::setName(const std::string& name) {
        name_ = name;
    }

    Node* Component::getOwner() const {
        return owner_;
    }

    void Component::setOwner(Node *owner) {
        owner_ = owner;
    }

    bool Component::isEnabled() const {
        return enabled_;
    }

    void Component::setEnabled(bool enable) {
        enabled_ = enable;
    }

    ComponentContainer::ComponentContainer(Node *object)
        : components_(nullptr)
        , owner_(object) {
    }

    ComponentContainer::~ComponentContainer(void) {
        SAFE_DELETE(components_);
    }

    Component* ComponentContainer::get(const std::string& name) const {
        Component* ret = nullptr;
        do {
            if(nullptr == components_)break;
            ret = components_->at(name);
        } while (0);
        return ret;
    }

    bool ComponentContainer::add(Component *com) {
        bool ret = false;
        do {
            if (components_ == nullptr) {
                components_ = new (std::nothrow) HObjectMap<std::string, Component*>();
            }

            Component *component = components_->at(com->getName());
            if(component) break;
            com->setOwner(owner_);
            components_->insert(com->getName(), com);
            com->onAdd();
            ret = true;
        } while(0);
        return ret;
    }

    bool ComponentContainer::remove(const std::string& name) {
        bool ret = false;
        do {
            if(!components_) break;

            auto iter = components_->find(name);
            if(iter == components_->end()) break;

            auto com = iter->second;
            com->onRemove();
            com->setOwner(nullptr);

            components_->erase(iter);
            ret = true;
        } while(0);
        return ret;
     }

    bool ComponentContainer::remove(Component *com) {
        bool ret = false;
        do {
            if(!components_) break;

            for (auto iter = components_->begin(); iter != components_->end(); ++iter) {
                if (iter->second == com) {
                    com->onRemove();
                    com->setOwner(nullptr);
                    components_->erase(iter);
                    break;
                }
            }
            ret = true;
        } while(0);

        return ret;
    }

    void ComponentContainer::removeAll() {
        if (components_ != nullptr) {
            for (auto iter = components_->begin(); iter != components_->end(); ++iter) {
                iter->second->onRemove();
                iter->second->setOwner(nullptr);
            }

            components_->clear();
            SAFE_DELETE(components_);

            dynamic_cast<Node *>(owner_)->unscheduleUpdate();
        }
    }

    void ComponentContainer::alloc(void) {
        components_ = new (std::nothrow) HObjectMap<std::string, Component*>();
    }

    void ComponentContainer::visit(float delta) {
        if (components_ != nullptr) {
            SAFE_RETAIN(owner_);
            for (auto iter = components_->begin(); iter != components_->end(); ++iter) {
                iter->second->update(delta);
            }
            SAFE_RELEASE(owner_);
        }
    }

    bool ComponentContainer::isEmpty() const {
        return (components_ == nullptr || components_->empty());
    }

    void ComponentContainer::onEnter() {
        for (auto iter = components_->begin(); iter != components_->end(); ++iter) {
            iter->second->onEnter();
        }
    }

    void ComponentContainer::onExit() {
        for (auto iter = components_->begin(); iter != components_->end(); ++iter) {
            iter->second->onExit();
        }
    }
}
