// #include "Entity.hpp"

// #include <iostream>

// Entity::Entity(int32_t  ID) : m_ID(ID), m_playable(false), m_name("none"), m_components() {}

// // Dont know if this is the correct way to free components
// Entity::~Entity() {
//   for (uint32_t 32_t i = 0; i < m_components.size(); i++) {
//     delete m_components[i];
//   }
//   m_components.clear();
// }

// int32_t  Entity::getID() { return m_ID; }

// template<typename T>
// bool Entity::addComponent(T *component) {
//   if (!hasComponent(component->getComponentType())) {
//     m_components.push_back(component);
//     return true;
//   }
//   return false;
// }

// bool Entity::hasComponent(ComponentType type) {
//   for (auto &c : m_components) {
//     if (c->getComponentType() == type) {
//       return true;
//     }
//   }
//   return false;
// }

// void Entity::removeComponent(ComponentTypeEnum removeComponent) {
//   for (uint32_t 32_t i = 0; i < m_components.size(); i++) {
//     if (m_components[i]->getComponentType() == removeComponent) {
//       delete m_components[i];
//       m_components.erase(m_components.begin() + i);
//       return;
//     }
//   }
// }

// Component *Entity::getComponent(ComponentTypeEnum type) {
//   for (auto *c : m_components) {
//     if ((c != nullptr) && c->getComponentType() == type) {
//       return c;
//     }
//   }

//   return nullptr;
// }
