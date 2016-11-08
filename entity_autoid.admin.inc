<?php
/**
 * @file
 * Configuration forms.
 *
 * Form to add a new schema.
 * List, view and delete existing schemas.
 */

/**
 * Implements hook_form().
 */
function entity_autoid_overview_form($form, &$form_state) {
  $header = array(
    'sid' => t('Schema ID'),
    'entity_type' => t('Entity Type'),
    'bundle' => t('Bundle'),
    'format' => t('Date format'),
    'seed_format' => t('Seed format'),
    'id_value' => t('Current ID'),
    'id_seed' => t('Current seed'),
    'op' => t('Operation'),
  );

  $rows = array();
  $schema_list = db_query('SELECT * FROM {entity_flow_id_schema}')->fetchAllAssoc('sid');

  foreach ($schema_list as $schema_id => $info) {
    $row['sid'] = $schema_id;
    $row['entity_type'] = $info->entity_type;
    $row['bundle'] = $info->bundle;
    $row['format'] = $info->format;
    $row['seed_format'] = $info->seed_format;
    $row['id_value'] = empty($info->id_value) ? 'N/A' : $info->id_value;
    $row['id_seed'] = empty($info->id_seed) ? 'N/A' : $info->id_seed;
    $row['op'] = l(t('Edit'), 'admin/config/entity-autoid/' . $info->sid . '/edit');
    $rows[$schema_id] = $row;
  }
  $form['schema_select'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No Schema.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  return $form;
}

/**
 * Submit handler for date_serial_edit_schema_form.
 */
function entity_autoid_overview_form_submit($form, &$form_state) {
  //cache_clear_all('entity_flow_id_schema', 'cache');
  $select = $form_state['values']['schema_select'];
  if (db_query('DELETE FROM {entity_flow_id_schema} WHERE sid IN (:sids)',array(':sids' => $select))) {
    drupal_set_message(t("Schema @ids had been deleted.",
      array(
        '@ids' => implode(',', $select),
      )));
  }
}


/**
 * Implements hook_form().
 */
function entity_autoid_form($form, &$form_state, $autoid = NULL) {
  $form_state['autoid'] = $autoid;
  $entity_info = entity_get_info();
  $form['#prefix'] = '<div id="entity-autoid-form-wrapper">';
  $form['#suffix'] = '</div>';

  $form['entity_type'] = array(
    '#type' => 'select',
    '#title' => t('Module Name'),
    '#options' => drupal_map_assoc(array_keys($entity_info)),
    '#required' => TRUE,
    '#default_value' => !empty($autoid) ? $autoid->entity_type : NULL,
    '#description' => t('Select the entity type.'),
    '#ajax' => array(
      'callback' => 'entity_autoid_form_ajax_callback',
      'wrapper' => 'entity-autoid-form-wrapper',
      'method' => 'replace',
    ),
  );
  if ($autoid) {
    $form['sid'] = array(
      '#type' => 'hidden',
      '#value' => $autoid->sid,
    );
    $entity_info = entity_get_info($autoid->entity_type);
    $form['bundle'] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(array_keys($entity_info['bundles'])),
      '#default_value' => !empty($autoid) ? $autoid->bundle : NULL,
      '#required' => TRUE,
      '#title' => t('Bundle'),
      '#description' => t('The prefix for the new IDs'),
    );
  }
  //ajax proceed.
  if (isset($form_state['values']) && !empty($form_state['values']['entity_type'])) {
    $entity_info = entity_get_info($form_state['values']['entity_type']);
    $form['bundle'] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(array_keys($entity_info['bundles'])),
      '#required' => TRUE,
      '#title' => t('Bundle'),
      '#description' => t('The prefix for the new IDs'),
    );
  }

  $form['format'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Format'),
    '#size' => 35,
    '#default_value' => 'Ymd',
    '#maxlength' => 128,
    '#default_value' => !empty($autoid) ? $autoid->format : NULL,
    '#description' => t('Format, token support'),
  );
  if (isset($form_state['values']) && !empty($form_state['values']['entity_type']) && module_exists('token')) {
    $token_types = array($form_state['values']['entity_type']);
    $form['token_tree'] = array(
      '#theme' => 'token_tree',
      '#token_types' => $token_types,
    );
  }
  else {
    $form['token_tree'] = array(
      '#markup' => '<p>' . t('Enable the <a href="@drupal-token">Token module</a> to view the available token browser.', array('@drupal-token' => 'http://drupal.org/project/token')) . '</p>',
    );
  }
  $form['seed_format'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Seed format'),
    '#size' => 35,
    '#default_value' => '%04d',
    '#maxlength' => 32,
    '#default_value' => !empty($autoid) ? $autoid->seed_format : '%04d',
    '#description' => t('Format to process the new IDs. go to http://php.net/SprintF for details.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}
/**
 * Ajax callback.
 */
function entity_autoid_form_ajax_callback(&$form, &$form_state) {
  $form_state['storage']['entity_type'] = $form_state['values']['entity_type'];
  return $form;
}
/**
 * Submit handler for date_serial_create_schema_form.
 */
function entity_autoid_form_submit($form, &$form_state) {
  //cache_clear_all('date_serial_schema', 'cache');
  form_state_values_clean($form_state);
  $entity_type = $form_state['values']['entity_type'];
  $bundle = $form_state['values']['bundle'];
  //create check
  $res = db_select('entity_flow_id_schema', 'f')
    ->fields('f')
    ->condition('entity_type', $entity_type)
    ->condition('bundle', $bundle)
    ->execute();
  $count = $res->rowCount();
  if ($count && !$form_state['autoid']) {
    drupal_set_message(t('@entity_type:@bundle exist', array('@entity_type' => $entity_type,'@bundle' => $bundle)));
    return;
  }
  //insert or update?
  if ($form_state['autoid']) {
    drupal_write_record('entity_flow_id_schema', $form_state['values'], 'sid');
    drupal_set_message(t('Schema had been updated.'));
  }
  else {
    drupal_write_record('entity_flow_id_schema', $form_state['values']);
    drupal_set_message(t('New ID Schema had been created.'));
  }
  $form_state['redirect'] = 'admin/config/entity-autoid/list';
}