#pragma once
#include <utils/uuid.h>

#include <QString>
#include <variant>

#include "CardItemDetailModel.h"
#include "LoginItemDetailModel.h"

using VaultItemDetailsVariant =
    std::variant<LoginItemDetailModel, CardItemDetailModel>;

class VaultItemModel {
 public:
  const QString& getName() const noexcept { return m_name; }
  const QString& getNote() const noexcept { return m_note; }
  const QString& getId() const noexcept { return m_id; }
  const VaultItemDetailsVariant& getDetails() const noexcept {
    return m_details;
  }
  bool isFavorite() const noexcept { return m_isFavorite; }
  bool isDeleted() const noexcept { return m_isDeleted; }

  VaultItemDetailsVariant& getDetails() noexcept { return m_details; }

  template <class T>
  T* getDetails() {
    return std::get_if<T>(&m_details);
  }

  template <class T>
  const T* getDetails() const {
    return std::get_if<T>(&m_details);
  }

  template <class T>
  void resetType() {
    m_details = T();
  }

  void generateId() {
    m_id = QString::fromStdString(ikea400::uuid::rand().toString());
  }

  void setId(const QString& id) { m_id = id; }
  void setName(const QString& name) { m_name = name; }
  void setNote(const QString& note) { m_note = note; }
  void setIsFavorite(bool isFavorite) { m_isFavorite = isFavorite; }
  void setIsDeleted(bool isDeleted) { m_isDeleted = isDeleted; }

  void set(VaultItemDetailsVariant&& details) {
    m_details = std::move(details);
  }

 private:
  VaultItemDetailsVariant m_details;
  QString m_name;
  QString m_note;
  QString m_id;
  bool m_isFavorite{false};
  bool m_isDeleted{false};
};