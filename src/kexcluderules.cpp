/*
 *   License:	LGPL - See file COPYING.LIB for details.
 *   Author:	Stefan Hundhammer <sh@suse.de>
 *              Joshua Hodosh <kdirstat@grumpypenguin.org>
 */

#include "kexcluderules.h"
#include <QDebug>

#define VERBOSE_EXCLUDE_MATCHES 1

using namespace KDirStat;

KExcludeRule::KExcludeRule(const QRegExp &regexp)
    : _regexp(regexp), _enabled(true) {
  // NOP
}

KExcludeRule::~KExcludeRule() {
  // NOP
}

bool KExcludeRule::match(const QString &text) {
  if (text.isEmpty() || !_enabled)
    return false;

  return _regexp.exactMatch(text);
}

KExcludeRules::~KExcludeRules() {
  foreach (KExcludeRule *rule, _rules)
    delete rule;
}

KExcludeRules *KExcludeRules::excludeRules() {
  static KExcludeRules *singleton = 0;

  if (!singleton) {
    singleton = new KExcludeRules();
  }

  return singleton;
}

void KExcludeRules::add(KExcludeRule *rule) {
  if (rule)
    _rules.append(rule);
}

bool KExcludeRules::match(const QString &text) {
  if (text.isEmpty())
    return false;

  foreach (KExcludeRule *rule, _rules) {
    if (rule->match(text)) {
#if VERBOSE_EXCLUDE_MATCHES

      qDebug() << text << " matches exclude rule " << rule->regexp().pattern()
               << Qt::endl;

#endif
      return true;
    }
  }

  return false;
}

const KExcludeRule *KExcludeRules::matchingRule(const QString &text) {
  if (text.isEmpty())
    return NULL;

  foreach (KExcludeRule *rule, _rules) {
    if (rule->match(text))
      return rule;
  }

  return 0;
}

