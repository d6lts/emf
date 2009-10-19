<?php
// $Id

/**
 * @file
 * Helper functions used by all files.
 *
 * @author Davy Van Den Bremt
 */

function emf_helpers_parse_field_values($lid) {
  global $user;

  $values = array();

  $list = emf_db_get_list($lid);

  foreach ($list->field_mapping as $field) {
    // php mapping
    if ($field->mapping_type == 'php') {
      $code = $field->mapping_value;
      $values[$field->key] = drupal_eval($code);
    }

    // profile mapping
    elseif ($field->mapping_type == 'profile' && module_exists('profile')) {
      profile_load_profile($user);

      if ($user->{$field->mapping_value}) {
        $values[$field->key] = $user->{$field->mapping_value};
      }
    }

    // token mapping
    elseif ($field->mapping_type == 'token' && module_exists('token')) {
      if (module_exists('profile')) {
        profile_load_profile($user);
      }

      $token_pattern = $field->mapping_value;
      $values[$field->key] = token_replace($token_pattern, 'user', $user);
    }
  }

  return $values;
}

/**
 * Subscribe a user to a list.
 *
 * @param $mail
 *   String; E-mail address to subscribe
 * @param $lid
 *   String; List ID of the list to subscribe to.
 * @param $fields
 *   Array; Array of custom field values. Key is field. Value is value for the field.
 */
function emf_helpers_subscribe($mail, $lid, $fields) {
  emf_db_save_request('subscribe', $mail, $lid, $fields);
  emf_db_save_subscription($mail, $lid);
}

/**
 * Unsubscribe a user from a list.
 *
 * @param $mail
 *   String; E-mail address to unsubscribe
 * @param $lid
 *   String; List ID of the list to unsubscribe from.
 */
function emf_helpers_unsubscribe($mail, $lid) {
  emf_db_save_request('unsubscribe', $mail, $lid);
  emf_db_delete_subscription($mail, $lid);
}

/**
 * Check if a certain sync action is allowed to happen.
 *
 * @param $type
 *   String; Type of sync, used in the variable name.
 * @return
 *   Boolean; TRUE if sync can happen; FALSE if not.
 */
function _emf_allow_sync($type) {
  $frequency = variable_get('emf_sync'. $type .'_interval', 0);
  $last_sync = variable_get('emf_last_sync_'. $type, 0);

  if ($frequency > -1 && ($_SERVER['REQUEST_TIME'] - $last_sync > $frequency)) {
    return TRUE;
  }

  return FALSE;
}