#include "IconProvider.h"

#include "../core/FaviconManager.h"

IconProvider::IconProvider(QObject* parent)
    : QObject(parent),
      m_defaultLoginIcon(":/icons/icons/login.svg"),
      m_defaultCardIcon(":/icons/icons/card.svg"),
      m_defaultIdentityIcon(":/icons/icons/idcard.svg"),
      m_defaultNoteIcon(":/icons/icons/note.svg")
{
  connect(FaviconManager::instance(), &FaviconManager::iconUpdated, this,
          &IconProvider::iconUpdated);
}

QIcon IconProvider::getItemIcon(ikea400::dto::VaultItemType type,
                                const QString& customIcon) const {
  switch (type) {
    case ikea400::dto::VaultItemType::Login: {
      if (!customIcon.isEmpty()) {
        return FaviconManager::instance()->getFaviconForUrl(customIcon.toStdString());
      }
      return m_defaultLoginIcon;
    } break;
    case ikea400::dto::VaultItemType::Card: {
      if (!customIcon.isEmpty()) {
        return getCardItemIcon(customIcon);
      }
      return m_defaultCardIcon;
    } break;
    case ikea400::dto::VaultItemType::Identity: {
      return m_defaultIdentityIcon;
    } break;
    case ikea400::dto::VaultItemType::Note: {
      return m_defaultNoteIcon;
    } break;

    default:
      break;
  }
  return {};
}

QIcon IconProvider::getItemIcon(
    const VaultItemSummaryModel& itemSummary) const {
  return getItemIcon(itemSummary.getType(), itemSummary.getCustomIconStr());
}

QIcon IconProvider::getItemIcon(const VaultItem& item) const {
  return getItemIcon(item.getType(), QString::fromStdString(item.getCustomIcon()));
}

QIcon IconProvider::getCardItemIcon(const QString& brand) const {

    if (brand.compare("visa", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/visa.svg");
  } else if (brand.compare("mastercard", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/mastercard.svg");
  } else if (brand.compare("amex", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/amex.svg");
  } else if (brand.compare("discover", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/discover.svg");
  } else if (brand.compare("jcb", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/jcb.svg");
  } else if (brand.compare("unionpay", Qt::CaseInsensitive) == 0) {
    return QIcon(":/icons/icons/unionpay.svg");
  }

  return {};
}
