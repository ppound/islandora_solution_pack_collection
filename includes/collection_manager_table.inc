<?php

/**
 * @file
 * collection_manager_table.inc
 */

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $collection_pid parameter
 * @param string $collection_pid
 * @return array
 */
function islandora_collection_table($object) {
  $query = 'select $object $title from <#ri>
              where  ($object <info:fedora/fedora-system:def/relations-external#isMemberOf> <info:fedora/' . $object->id . '>
              or $object <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/' . $object->id . '>)
              and $object <dc:title> $title';
  $results = $object->repository->ri->itqlQuery($query, 'unlimited', '0');
  $keys = array();
  $objects = array();
  foreach ($results as $result) {
    $objects[$result['object']['value']] = $result['title']['value'];
    $keys[] = $result['object']['value'];
  }
  $rows = array();
  foreach ($objects as $key => $object) {
    $rows[$key] = array(
      '#pid' => $key,
      'pid' => array('#value' => l($key, 'islandora/object/' . $key)),
      'title' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $object,
          '#href' => 'islandora/object/' . $key,
          ),
        ),
    );
  }

  $header = array(
    'title' => array('data' => t('Title')),
    );

  if (!$rows) {
    return;
  }

  $table = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
  );

  return $table;

}

/**
 * themes the form table.
 *
 * @param array $element Drupal Form Element.
 * @return string
 */
function theme_islandora_basic_collection_management_form_table(array $element) {
  $rows = array();
  foreach (element_children($element['rows']) as $child) {
    $setting = $element['rows'][$child];
    $pid = $setting['#pid'];
    $fields = array(
      drupal_render($element['selections'][$pid]) // First field is a checkbox
    );
    foreach (element_children($setting) as $property) {
      $field = $setting[$property];
      $fields[] = drupal_render($field);
    }
    $rows[] = array(
      'data' => $fields,
      'class' => isset($setting['#attributes']['class']) ? $setting['#attributes']['class'] : NULL
    );
  }
  $attributes = isset($element['#id']) ? array('id' => $element['#id']) : NULL;
  return theme_table($element['#header'], $rows, $attributes);
}

function get_collections_as_option_array() {
  $restricted = variable_get('islandora_namespace_restriction_enforced', FALSE);
  $allowed_string = variable_get('islandora_pids_allowed', 'default: demo: changeme: islandora:');
  $namespaces = explode(':', $allowed_string);

  $connection = islandora_get_tuque_connection();
  if (!$connection) {
    return array();
  }
  $query = 'select $object $title from <#ri>
                            where ($object <fedora-model:label> $title
                              and $object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:collectionCModel>
                              and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
                            order by $title';
  $results = $connection->repository->ri->itqlQuery($query, 'unlimited', '0');
  foreach ($namespaces as $namespace) {
    $trimmed_names[] = trim($namespace);
  }
  $options = array();
  foreach ($results as $item) {   //removes  blanks
    $namespace = explode(':', $item['object']['value']);
    $namespace = trim($namespace[0]);
    if (!$restricted || in_array($namespace, $trimmed_names)) {
      $options[$item['object']['value']] = $item['title']['value'];
    }
  }
  unset($options['islandora:ContentModelCollection']);
  return $options;
}
