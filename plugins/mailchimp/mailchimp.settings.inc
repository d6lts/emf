<?php
// $Id

/**
 * @file
 * Settings functions and callbacks.
 *
 * @author Davy Van Den Bremt
 */

function mailchimp_settings() {
  $form = array();

  // info
  $form['mailchimp_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#description' => t('Your MailChimp API Key.'),
    '#default_value' => variable_get('mailchimp_api_key', ''),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}