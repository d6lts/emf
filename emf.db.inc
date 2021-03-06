<?php

/**
 * @file
 * Database wrapper functions.
 *
 * @author Davy Van Den Bremt
 */

/**
 * Fetch requests from database;
 *
 * @return
 *   Array; List of requests.
 */
function emf_db_get_requests() {
  $requests = array();
  $result = db_query("SELECT * FROM {emf_request} ORDER BY timestamp ASC");
  while ($request = db_fetch_object($result)) {
    $request->fields = empty($request->fields) ? array() : unserialize($request->fields);
    $requests[$request->mail] = $request;
  }
  return $requests;
}

/**
 * Save a request to database.
 *
 * @param $type
 *   String; Type of request; 'subscribe' or 'unsubscribe'.
 * @param $mail
 *   String; The mail address the request is for.
 * @param $lid
 *   String; The list id the request is for.
 * @param $fields
 *   Array; Values for fields to subscribe.
 */
function emf_db_save_request($type, $mail, $lid, $fields = array(), $fails = 0) {
  $exists = db_result(db_query("SELECT count(mail) FROM {emf_request} WHERE mail = '%s' AND lid = '%s'", $mail, $lid));
  if ($exists) {
    db_query("UPDATE {emf_request} SET type = '%s', fields = '%s', timestamp = %d, fails = %d WHERE mail = '%s' AND lid = '%s'", array($type, serialize($fields), time(), $fails, $mail, $lid));
  }
  else {
    db_query("INSERT INTO {emf_request} (type, mail, lid, fields, timestamp, fails) VALUES ('%s', '%s', '%s', '%s', %d, %d)", array($type, $mail, $lid, serialize($fields), time(), $fails));
  }
}


/**
 * Delete a request from database.
 *
 * @param $mail
 *   String; The mail address the request is for.
 * @param $lid
 *   String; The list id the request is for.
 */
function emf_db_delete_request($mail, $lid) {
  db_query("DELETE FROM {emf_request} WHERE mail = '%s' AND lid = '%s'", $mail, $lid);
}

/**
 * Fetch subscriptions for a given address.
 *
 * @param $mail
 *   String; The mail address to fetch subscriptions for.
 * @result
 *   Array; Array of list id's of the lists the address is subscribed to.
 */
function emf_db_get_subscriptions($mail) {
  $lists = array();
  $result = db_query("SELECT lid FROM {emf_subscription} WHERE mail = '%s'", $mail);
  while ($lid = db_fetch_object($result)) {
    $lists[] = $lid;
  }
  return $lists;
}

/**
 * Save a subscriptions for a given address to a given list.
 *
 * @param $mail
 *   String; The mail address to save the subscription for.
 * @param $lid
 *   String; The list id save the subscription for.
 */
function emf_db_save_subscription($mail, $lid) {
  $exists = db_result(db_query("SELECT mail FROM {emf_subscription} WHERE mail = '%s' AND lid = '%s'", $mail, $lid));
  if (!$exists) {
    db_query("INSERT INTO {emf_subscription} (mail, lid) VALUES ('%s', '%s')", $mail, $lid);
  }
}

/**
 * Delete a subscription for a given address to a given list.
 *
 * @param $mail
 *   String; The mail address to delete the subscription for.
 * @param $lid
 *   String; The list id delete the subscription for.
 */
function emf_db_delete_subscription($mail, $lid) {
  db_query("DELETE FROM {emf_subscription} WHERE mail = '%s' AND lid = '%s'", $mail, $lid);
}

/**
 * Fetch a list.
 *
 * @param $lid
 *   String; The list id of the list to fetch.
 * @result
 *   Object; The list object.
 */
function emf_db_get_list($lid = 0) {
  if (!$lid) return FALSE;
  $list = db_fetch_object(db_query("SELECT * FROM {emf_list} WHERE lid = '%s'", $lid));
  $list->field_mapping = empty($list->field_mapping) ? array() : unserialize($list->field_mapping);
  _emf_db_sort_field_mapping($list->field_mapping);

  $list->name_clean = strtr($list->name, array('!name_api' => $list->name_api));
  $list->name_action_clean = strtr($list->name_action, array('!name_api' => $list->name_api, '!name' => $list->name_clean));
  $list->name_action_long_clean = strtr($list->name_action_long, array('!name_api' => $list->name_api, '!name' => $list->name_clean));

  return $list;
}

function _emf_db_sort_field_mapping(&$mapping) {
  uasort($mapping, '_emf_db_field_mapping_sort');
}

function _emf_db_field_mapping_sort($a, $b) {
  if ($a->weight == $b->weight) return 0;
  return ($a->weight < $b->weight) ? -1 : 1;
}

/**
 * Fetch all lists.
 *
 * @param $active_only
 *   Boolean; TRUE to only fetch active lists. FALSE if all.
 * @result
 *   Array; Array of list objects.
 */
function emf_db_get_lists($active_only = FALSE) {
  $lists = array();

  // only fetch active lists
  $filter = '';
  if ($active_only) {
    $filter = ' WHERE status = 1';
  }

  $result = db_query("SELECT * FROM {emf_list}$filter");
  while ($row = db_fetch_object($result)) {
    $row->field_mapping = empty($row->field_mapping) ? array() : unserialize($row->field_mapping);
    _emf_db_sort_field_mapping($row->field_mapping);
    $lists[$row->lid] = $row;
    $lists[$row->lid]->name_clean = strtr($lists[$row->lid]->name, array('!name_api' => $lists[$row->lid]->name_api));
    $lists[$row->lid]->name_action_clean = strtr($lists[$row->lid]->name_action, array('!name_api' => $lists[$row->lid]->name_api, '!name' => $lists[$row->lid]->name_clean));
    $lists[$row->lid]->name_action_long_clean = strtr($lists[$row->lid]->name_action_long, array('!name_api' => $lists[$row->lid]->name_api, '!name' => $lists[$row->lid]->name_clean));
  }

  return $lists;
}

/**
 * Save a list.
 *
 * @param $lid
 *   String; List id of the list.
 * @param $name_api
 *   String; Name of the list on the API.
 * @param $status_api
 *   Integer; Status on the api of the list.
 * @param $name
 *   String; Site name for the list.
 * @param $status
 *   Integer; Status of the list.
 * @param $name_action
 *   String; Action name for the list.
 * @param $name_action_long
 *   String; Long action name for the list.
 * @param $description
 *   String; Description for the list.
 */
function emf_db_save_list($lid, $name_api, $status_api, $name, $status, $name_action, $name_action_long, $description, $language) {
  $exists = db_result(db_query("SELECT lid FROM {emf_list} WHERE lid = '%s'", $lid));
  if ($exists) {
    db_query("UPDATE {emf_list} SET name_api = '%s', status_api = %d, name = '%s', status = %d, name_action = '%s', name_action_long = '%s', description = '%s', language = '%s' WHERE lid = '%s'", $name_api, $status_api, $name, $status, $name_action, $name_action_long, $description, $language, $lid);
  }
  else {
    db_query("INSERT INTO {emf_list} (lid, name_api, status_api, name, status, name_action, name_action_long, description, language) VALUES ('%s', '%s', %d, '%s', %d, '%s', '%s', '%s', '%s')", $lid, $name_api, $status_api, $name, $status, $name_action, $name_action_long, $description, $language);
  }
}

/**
 * Delete a list and all its request and subscriptions.
 *
 * @param $lid
 *   String; List id of the list to delete.
 */
function emf_db_delete_list($lid) {
  db_query("DELETE FROM {emf_list} WHERE lid = '%s'", $lid);
  db_query("DELETE FROM {emf_request} WHERE lid = '%s'", $lid);
  db_query("DELETE FROM {emf_subscription} WHERE lid = '%s'", $lid);
  db_query("DELETE FROM {emf_list_roles} WHERE lid = '%s'", $lid);
}

/**
 * Save a field mapping for a given list.
 *
 * @param $lid
 *   String; List id of the list to save the mapping for.
 * @param $field_mapping
 *   Array; Field mapping to save.
 */
function emf_db_save_field_mapping($lid, $field_mapping = array()) {
  db_query("UPDATE {emf_list} SET field_mapping = '%s' WHERE lid = '%s'", serialize($field_mapping), $lid);
}