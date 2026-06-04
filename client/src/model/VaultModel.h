#pragma once

#include <QString>
#include <string>

class VaultModel {
 public:
  explicit VaultModel(const std::string& n, const std::string& id,
                      const std::string& desc)
      : m_name(QString::fromStdString(n)),
        m_id(QString::fromStdString(id)),
        m_description(QString::fromStdString(desc)) {}

  const auto& getName() const noexcept { return m_name; }
  const auto& getId() const noexcept { return m_id; }
  const auto& getDescription() const noexcept { return m_description; }

 private:
  QString m_name;
  QString m_description;
  QString m_id;
};