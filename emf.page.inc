<?php

function emf_page($list = NULL) {
  module_load_include('inc', 'emf', 'emf.form');
  return drupal_get_form('emf_subscription_list_form', $list->lid);
}