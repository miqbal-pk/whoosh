<?php
/**
 * @file
 * model_platform_quicktabs_views.features.menu_custom.inc
 */

/**
 * Implements hook_menu_default_menu_custom().
 */
function model_platform_quicktabs_views_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-customer-profile-menu.
  $menus['menu-customer-profile-menu'] = array(
    'menu_name' => 'menu-customer-profile-menu',
    'title' => 'Customer profile menu',
    'description' => '',
    'language' => 'und',
    'i18n_mode' => 5,
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Customer profile menu');

  return $menus;
}
