#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include "BASE/HObject.h"

namespace GRAPH
{
    enum
    {
        kComponentOnEnter,
        kComponentOnExit,
        kComponentOnAdd,
        kComponentOnRemove,
        kComponentOnUpdate
    };

    class Component : public HObject
    {
    public:
        Component(void);

    public:
        virtual ~Component(void);
        virtual bool init();

        virtual void onEnter();
        virtual void onExit();
        virtual void onAdd();
        virtual void onRemove();
        virtual void update(float delta);
        virtual bool serialize(void* r);
        virtual bool isEnabled() const;
        virtual void setEnabled(bool b);
        static Component* create(void);

        const std::string& getName() const;
        void setName(const std::string& name);

        void setOwner(HObject *pOwner);
        HObject* getOwner() const;

    protected:
        HObject *_owner;
        std::string _name;
        bool _enabled;
    };

    class ComponentContainer
    {
    protected:
        ComponentContainer(HObject *pHObject);

    public:
        virtual ~ComponentContainer(void);

        virtual Component* get(const std::string& name) const;
        virtual bool add(Component *com);
        virtual bool remove(const std::string& name);
        virtual bool remove(Component *com);
        virtual void removeAll();
        virtual void visit(float delta);

        virtual void onEnter();
        virtual void onExit();

    public:
        bool isEmpty() const;

    private:
        void alloc(void);

    private:
        HObjectMap<std::string, Component*>* _components;
        HObject *_owner;

        friend class Node;
    };
}

#endif  // COMPONENT_H
