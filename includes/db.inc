<?php

/** Unit to perform various database functions.
 *
 */

/**
 * Helper function to create the search form filter choices for collections in workflow_django
 *
 * @param string $include_filter
 *   Filter to be applied to the SQL to include matches.
 * @param string $exclude_filter
 *   Filter to be applied to the SQL to exclude matches.
 * @return array
 *   Array of batch records - each array element is an array keyed on
 * batch.name and contains the "items count" and the
 * batch.name value for that batch.
 */
function upitt_islandora_digital_workflow_migration_get_batches($include_filter = '', $exclude_filter = '') {
  $link = upitt_workflow_get_databaselink('drlworkflow');

  $include_filter_like_string = str_replace("*", "%", $include_filter);
  if (strstr($include_filter, '*')) {
    $include_filter_where_clause = ($include_filter ? "b.`name` LIKE '" .$include_filter_like_string. "'" : '');
  }
  else {
    $include_filter_where_clause = ($include_filter ? "b.`name` = '" .$include_filter_like_string. "'" : '');  }

  $exclude_filter_like_string = str_replace("*", "%", $exclude_filter);
  if (strstr($exclude_filter, '*')) {
    $exclude_filter_where_clause = ($exclude_filter ? "b.`name` NOT LIKE '" .$exclude_filter_like_string. "'" : '');
  }
  else {
    $exclude_filter_where_clause = ($exclude_filter ? "b.`name` <> '" .$exclude_filter_like_string. "'" : '');
  }
  if ($include_filter || $exclude_filter) {
    $filter_where_clause = 'WHERE ';
    if ($include_filter_where_clause && $exclude_filter_where_clause) {
      $filter_where_clause .= $include_filter_where_clause . ' AND ' . $exclude_filter_where_clause;
    }
    else {
      // Get both values without checking -- it must be one clause at this point.
      $filter_where_clause .= $include_filter_where_clause . $exclude_filter_where_clause;
    }
  }
  else {
    $filter_where_clause = '';
  }

  $query = 'SELECT b.`name`, COUNT(bi.item_id) `items count` ' .
    'FROM `core_batch` b ' .
    'LEFT OUTER JOIN `core_batch_item` bi ON (bi.batch_id = b.id) ' .
    $filter_where_clause .
    ' GROUP BY b.id ORDER BY b.`name`';

  $result = mysqli_query($link, $query);
  if (!$result) {
    upitt_workflow_sql_error_die($link, $query);
  }
  $rows = array();
  while ($row = mysqli_fetch_assoc($result)) {
    $rows[$row['name']] = $row;
  }
  mysqli_close($link);
  return $rows;
}

/**
 * Database query function to return the matching batch as an array.
 *
 * @param string $batch_name
 *   The unique name of a batch record.
 * @return array
 *   An array of batch record field => values pairs.
 */
function upitt_islandora_digital_workflow_migration_get_batch($batch_name) {
  $link = upitt_workflow_get_databaselink('drlworkflow');
  $query = 'SELECT b.*, wfllb.*, wffc.`name` as `collection name`, wffc.`pid`, cc.`c_id` ' .
    'FROM `core_batch` b ' .
    'LEFT OUTER JOIN `wflocal_local_batch` wfllb ON (wfllb.batch_ptr_id = b.id) ' .
    'LEFT OUTER JOIN `core_collection` cc ON (cc.id = b.collection_id) ' .
    'LEFT OUTER JOIN `core_batch_item` cbi ON (cbi.batch_id = b.id) ' .
    'LEFT OUTER JOIN `wflocal_local_item_fedora_collections` wflifc ON (wflifc.local_item_id = cbi.item_id) ' .
    'LEFT OUTER JOIN `wflocal_fedora_collection` wffc ON (wffc.`id` = wflifc.fedora_collection_id) ' .
    'WHERE b.`name` = "' . $batch_name . '"';
  $result = mysqli_query($link, $query);
  if (!$result) {
    upitt_workflow_sql_error_die($link, $query);
  }
  $rows = array();
  while ($row = mysqli_fetch_assoc($result)) {
    $rows = $row;
  }
  mysqli_close($link);
  return $rows;
}

/**
 * Database query function to return the items related to a batch.
 *
 * @param string $batch_name
 *   The identifier field for a batch.
 * @return array
 *   Array of batch items keyed on the item's do_id - each is an array of field
 * values.
 */
function upitt_islandora_digital_workflow_migration_get_batch_item_records($batch_name) {
  $link = upitt_workflow_get_databaselink('drlworkflow');
  $query = 'SELECT ' .
    ' b.`name`, i.`do_id`, i.`name`, i.`type_id`, i.`property_owner_id`,' .
    ' i.`primary_collection_id`, bi.item_id, wflli.enumeration,' .
    ' wflli.date_issued, wflli.online_pub_date, wflli.copyright_status,' .
    ' wflli.publication_status, wflli.copyright_holder_name,' .
    ' wflli.permission_notes, wflli.voyager_id, wflli.ead_id, wflli.depositor,' .
    ' wflli.type_of_resource, wflli.genre, wflli.thumb_filename,' .
    ' wflli.digitization_responsibility, wflli.digitization_date,' .
    ' wffc.`name` as `collection name`, wffc.`pid`' .
    'FROM `core_batch` b ' .
    'LEFT OUTER JOIN `core_batch_item` bi ON (bi.batch_id = b.id) ' .
    'LEFT OUTER JOIN `core_item` i ON (i.id = bi.item_id) ' .
    'LEFT OUTER JOIN wflocal_local_item wflli ON (wflli.item_ptr_id = i.id) ' .
    'LEFT OUTER JOIN wflocal_local_item_fedora_collections wflifc ON (wflifc.local_item_id = i.`primary_collection_id`) ' .
    'LEFT OUTER JOIN wflocal_fedora_collection wffc ON (wffc.`id` = wflifc.fedora_collection_id) ' .
    'WHERE b.`name` = "' . $batch_name . '"';

  $result = mysqli_query($link, $query);
  if (!$result) {
    upitt_workflow_sql_error_die($link, $query);
  }
  $rows = array();
  while ($row = mysqli_fetch_assoc($result)) {
    $rows[$row['do_id']] = $row;
  }
  mysqli_close($link);
  return $rows;
}

/**
 * Database query function to return the transactions related to a batch item.
 *
 * @param string $item_id
 *   The identifier field for a item.
 * @return array
 *   Array of item transaction records keyed on the transaction id - each is an
 * array of transaction field values.
 */
function upitt_islandora_digital_workflow_migration_get_transaction_records($item_id) {
  $link = upitt_workflow_get_databaselink('drlworkflow');

  $query = 'SELECT t.`id`, t.`item_id`, t.`description`, ' .
      ' au.`username`, au.`first_name`, au.`last_name`, au.`email`, t.`timestamp` ' .
      'FROM `core_transaction` t ' .
      'LEFT OUTER JOIN auth_user au ON (au.id = t.user_id) ' .
      'WHERE t.`item_id` = "' . $item_id . '" ' .
      'ORDER BY t.`timestamp`';

  $result = mysqli_query($link, $query);
  if (!$result) {
    upitt_workflow_sql_error_die($link, $query);
  }
  $rows = array();
  while ($row = mysqli_fetch_assoc($result)) {
    $rows[$row['id']] = $row;
  }
  mysqli_close($link);
  return $rows;
}

/**
 * Will migrate the given $item array into new islandora_digital_workflow
 * batch item record.
 *
 * @param string $item
 *   The item array.
 * @param stdClass $idw_batch
 *   The new islandora_digital_workflow batch record as a stdClass object.
 * @param array $django_batch
 *   The DRL batch record - joined to the fedora collection and internal DRL
 * collection -- needed for naming the filename.
 * @param boolean $delete_existing_records
 *   Whether or not to delete any existing batch_items records before trying to
 * insert them.
 * @return mixed
 *   Either 0 when no batch_item is added, or the array that represents
 * the Islandora Digital Workflow batch_item record that was inserted.
 */
function upitt_islandora_digital_workflow_migration_create_item($item, $idw_batch, $django_batch, $delete_existing_records = TRUE) {
  /* $item keys:
   *   name, do_id, type_id, property_owner_id, primary_collection_id, item_id,
   * enumeration, date_issued, online_pub_date, copyright_status,
   * publication_status, copyright_holder_name, permission_notes, voyager_id,
   * ead_id, depositor, type_of_resource, genre, thumb_filename,
   * digitization_responsibility, digitization_date, collection name, pid
   *
   * islandora_digital_workflow_batch_items fields:
   *   batch_item_id, batch_id, identifier, title, filename, type_of_resource,
   * islandora_model, assigned_pid, mods, marc, progress, file_count, file_size
   */
  module_load_include('inc', 'islandora_digital_workflow', 'includes/db');
  if ($delete_existing_records) {
    $batch_item_id = islandora_digital_workflow_get_batch_item_id_by_identifier($item['do_id'], NULL, FALSE);
    if ($batch_item_id) {
      // This will also delete any related transaction records, "drush log",
      // and "process set" records related to each item.
      islandora_digital_workflow_delete_batch_item($batch_item_id);
    }
  }

  $islandora_model = upitt_islandora_digital_workflow_migration_get_islandora_model_from_type_id($item['type_id'], $django_batch['genre']);
  $idw_batch_item_array = array(
    // 'batch_item_id' => '', -- set by system -- need to look up what value was
    // generated by the autoincrement PK field.
    'batch_id' => $idw_batch->batch_id,
    'identifier' => $item['do_id'],
    'title' => $item['name'],
    'filename' => (($islandora_model == 'islandora:sp_large_image_cmodel') ?
      CST_DELIVERY_ROOT . '/' . substr($django_batch['c_id'], 0, 1) . '/' .
      $django_batch['c_id'] . '/' . $idw_batch->batch_name . '/' .
      $item['do_id'] . '/' . $item['thumb_filename'] : ''),
    'type_of_resource' => $item['type_of_resource'],
    'islandora_model' => $islandora_model,
    'assigned_pid' => 'pitt:' . $item['do_id'],
    'mods' => '',
    'marc' => '',
    'progress' => 'Migrate New',
    'file_count' => 0,
    'file_size' => 0
  );

  // Based on the model, if the item is :
  // islandora:newspaperIssueCModel:
  //   1 - potentially parse the title ($item['name']) to see if we can
  //       derive the "volume_number" and "issue_number".
  //   2 - populate the "issued_date" field with "date_issued".
  if ($islandora_model == 'islandora:newspaperIssueCModel') {
    $parsed_title_results = upitt_islandora_digital_workflow_migration_parse_title_for_volume_and_issue($item['name']);
    $volume_number = $parsed_title_results['volume'];
    $issue_number = $parsed_title_results['issue'];
    $issued_date = ($item['date_issued'] == 'NULL') ? 'NULL' : $item['date_issued'];
    $date_parts = explode("-", $issued_date);
    if (count($date_parts) == 1) {
      $issued_date .= '-01-01';
    }
    elseif (count($date_parts) == 2) {
      $issued_date .= '-01';
    }
  }
  else {
    $issued_date = '';
    $volume_number = $issue_number = '';
  }

  if ($issued_date) {
    $idw_batch_item_array['issued_date'] = $issued_date;
  }
  if ($volume_number) {
    $idw_batch_item_array['volume_number'] = $volume_number;

  }
  if ($issue_number) {
    $idw_batch_item_array['issue_number'] = $issue_number;
  }
  upitt_islandora_digital_workflow_migration_iconv_array($idw_batch_item_array);
  // Insert that record and then load that record to return it to the calling code.
  db_query("SET NAMES utf8");
  db_insert('islandora_digital_workflow_batch_items')
      ->fields($idw_batch_item_array)
      ->execute();

  $query = db_select('islandora_digital_workflow_batch_items', 'bi');
  $query->fields('bi');
  $query->condition('bi.identifier', $item['do_id']);
  $results = $query->execute();
  $rows = $results->fetchAll();
  $return = array();
  foreach ($rows as $row) {
    $return = $row;
  }
  return $return;
}


/**
 * Code to delete an existing batch record only.
 * 
 * This does not delete related batch_item or transactions records -- see the
 * call from within upitt_islandora_digital_workflow_migration_create_item that
 * calls the workflow function islandora_digital_workflow_delete_batch_item.
 *
 * @param string $batch_name
 *   Name of the batch to delete.
 */
function upitt_islandora_digital_workflow_migration_delete_batch($batch_name) {
  module_load_include('inc', 'islandora_digital_workflow', 'includes/db');
  // 1. get the current islandora_digital_workflow_batch record that matches
  // the batch_name.
  // 2. iterate through all the items related to this batch.
  //   a. delete any transactions related to the batch_item.
  //   b. delete the batch_item record.
  // 3. delete any transactions related to the batch itself.
  // 4. delete the batch record.
  $idw_batch = islandora_digital_workflow_find_batch_records_by_name($batch_name);
  if (is_array($idw_batch)) {
    $batch_record_object = array_pop($idw_batch);
    if (is_object($batch_record_object)) {
      // Delete the drupal node related to the batch.
      $nid = $batch_record_object->nid;
      if ($nid) {
        node_delete($nid);
      }
      db_delete('islandora_digital_workflow_batch')
        ->condition('batch_id', $batch_record_object->batch_id)
        ->execute();
    }
  }
}


/**
 * Will migrate the given $transaction array into new islandora_digital_workflow
 * batch item transaction record.
 *
 * @param string $transaction
 *   The transaction array.
 * @param string $idw_batch_item
 *   The item object.
 * @return mixed
 *   Either 0 when no transaction is being added, or the array that represents
 * the Islandora Digital Workflow transactions record that was inserted.
 */
function upitt_islandora_digital_workflow_migration_create_transaction($transaction, $idw_batch_item) {
  global $batch_item_ids;
  // can safely ignore all transactions that have one of the following description values
  $skip_descriptions = array("make thumnail performed", "ocr performed", "problem reported", "problem resolved",
    "unknown", "make jpg completed", "startScanFoldouts", "make DLXS textclass derivatives completed",
    "stopScanFoldouts", "make zoomify completed", "startScanColor", "stopScanColor",
    "stopProblem", "resetItemStatus", "make xml completed", "make macrob csv completed",
    "make pdf completed", "presolve", "preport", "exported files from repository to problems directory",
    "startBR", "startScanGray", "startMeta", "stopMeta", "stopBR", "stopScanGray",
    "deliver to webserver completed", "loadRecord", "make jpg2000 completed", "reset status",
    "prepForMetadata", "make thumbnail completed", "ingestIntoRepository", "ocr completed",
    "prepare macrob files completed", "ocr", "delivered", "xml", "derivatives",
    "migrated record from 1st gen workflow", "item record created", "ingest completed");
  if ((array_search($transaction['description'], $skip_descriptions) === FALSE)) {
    $map_actions = array(
      "check-out performed" => 4,
      "scan performed" => 2,
      "setReScan" => 5,
      "structural metadata performed" => 16,
      "checkOut" => 4,
      "release online completed" => 14,
      "checkIn" => 1,
      "online" => 14,
      "structural metadata completed" => 16,
      "prep for bodybuilder completed" => 16,
      "check-in completed" => 1,
      "scan completed" => 2,
      "check-out completed" => 4
    );
    $idw_transaction_array = array(
      'action_id' => $map_actions[$transaction['description']],
      'batch_id' => NULL,
      'batch_item_id' => $idw_batch_item->batch_item_id,
      'drupal_uid' => upitt_islandora_digital_workflow_migration_get_drupal_uid($transaction['email']),
      'timestamp' => $transaction['timestamp'],
    );
    // Insert that record and just return the array to the calling code.
    db_query("SET NAMES utf8");
    db_insert('islandora_digital_workflow_transactions')
        ->fields($idw_transaction_array)
        ->execute();

    return $idw_transaction_array;
  }
  else {
    return 0;
  }
}

/**
 * Will migrate the given $django_batch array into new islandora_digital_workflow batch record
 *
 * @param string $django_batch
 *   The batch array.
 * @param boolean $delete_existing_records
 *   Whether or not to delete any matching islandora_digital_workflow record
 * @param array $tid_priority_mapping
 *   The array of all taxonomy terms for the "workflow_batch_priority_vocab"
 * vocabulary (term tid and term name).
 * @return mixed
 *   Either 0 when no batch is added, or the array that represents
 * the Islandora Digital Workflow batch record that was inserted.
 */
function upitt_islandora_digital_workflow_migration_create_batch($django_batch, $delete_existing_records = TRUE, $tid_priority_mapping = array()) {
  /* QUESTIONS about mapping some django core_batch fields
   *   property_owner_id :: kind of like depositor, requires lookup table
   *     core_property_owner.
   *   source_id :: batch name identifier collection string like "AIS.2012.04", 
   *     "hpicusc", or "stalinka", requires wflocal_source_collection.
   *
   * MAPPED django_batch fields:
   *   (core_batch fields)
   *   name, type_id, description, item_count, date, active,
   *   (wflocal_local_batch fields)
   * priority, is_request, requestor, request_due_date,
   * ead_id, copyright_status, publication_status, copyright_holder_name,
   * depositor, type_of_resource, genre,
   *   (wflocal_fedora_collection fields)
   * (collection) pid,
   *
   * UNMAPPED django_batch fields:
   *   (core_batch fields)
   *   collection_id, property_owner_id, 
   * sequence_id, 
   *   (wflocal_local_batch fields)
   *   {source_id}, condition_handling, file_type, file_naming, resolution_ppi,
   * image_type_bit_depth, target_size, edge_treatment, use_color_target,
   * blank_missing_treatment, image_editing_treatment,
   * structural_metadata_treatment, voyager_id, permission_notes
   *   (wflocal_fedora_collection fields)
   * collection name, pid,
   *   (core_collection fields)
   * c_id
   *
   * islandora_digital_workflow_batch fields:
   *   batch_id, batch_name, islandora_model, batch_description,
   * batch_priority_tid, is_batch_active, is_batch_request, batch_requestor,
   * batch_request_due_date, default_ead_id, object_count,
   * uploaded_batch_items_file, ingest_namespace, mapto_collections,
   * mapto_site_id_values, ocr_language, default_depositor,
   * default_type_of_resource, default_genre, batch_default_CR_status,
   * batch_default_pub_status, batch_default_CR_holder, nid,
   * workflow_sequence_id, webform_submission_sid, progress, user, lastmod,
   * created
   */
  if ($delete_existing_records) {
    upitt_islandora_digital_workflow_migration_delete_batch($django_batch['name']);
  }
  $islandora_model = upitt_islandora_digital_workflow_migration_get_islandora_model_from_type_id($django_batch['type_id'], $django_batch['genre']);
  $priority_taxonomy_tid = upitt_islandora_digital_workflow_migration_get_priority($django_batch['priority'], $tid_priority_mapping);

  $idw_batch_array = array(
    'batch_name' => $django_batch['name'],
    'islandora_model' => $islandora_model,
    'batch_description' => $django_batch['description'],
    'batch_priority_tid' => $priority_taxonomy_tid,
    'mapto_collections' => $django_batch['pid'],
    'object_count' => $django_batch['item_count'],
    'default_type_of_resource' => $django_batch['type_of_resource'],
    'default_genre' => $django_batch['genre'],
    'default_depositor' => $django_batch['depositor'],
    'batch_default_CR_status' => $django_batch['copyright_status'],
    'batch_default_pub_status' => $django_batch['publication_status'],
    'batch_default_CR_holder' => $django_batch['copyright_holder_name'],
    'default_ead_id' => $django_batch['ead_id'],
    'is_batch_request' => $django_batch['is_request'],
    'batch_requestor' => $django_batch['requestor'],
    'batch_request_due_date' => (($django_batch['request_due_date']) ? strtotime($django_batch['request_due_date']) : NULL),
    'is_batch_active' => $django_batch['active'],
    'user' => 'upitt_islandora_digital_workflow_migration',
    'created' => $django_batch['date'],
  );
  // Save this array as a new record.
  // run a query to get the batch array (includes the field
  // `islandora_digital_workflow`.batch_id which is needed by the transaction
  // and item records that need to be related to it.
  db_query("SET NAMES utf8");
  db_insert('islandora_digital_workflow_batch')
      ->fields($idw_batch_array)
      ->execute();

  $query = db_select('islandora_digital_workflow_batch', 'b');
  $query->fields('b');
  $query->condition('b.batch_name', $django_batch['name']);
  $results = $query->execute();
  $rows = $results->fetchAll();
  $return = array();
  foreach ($rows as $row) {
    $return = $row;
  }

  return $return;
}

/**
 * Loops through all array elements and sets the value to the iconv value
 * to return all values as utf-8.
 *
 * @param array $idw_batch_item_array
 *   Array of name-value pairs for record field = value.
 */
function upitt_islandora_digital_workflow_migration_iconv_array(&$idw_batch_item_array) {
  foreach ($idw_batch_item_array as $key => $value) {
    $idw_batch_item_array[$key] = iconv("windows-1252", 'UTF-8', $value);
  }
}

function upitt_islandora_digital_workflow_migration_parse_title_for_volume_and_issue($title) {
  // parse the title value and return the volume and (issue) number.
  $uctitle = strtoupper($title);
  $has_vol = strstr($uctitle, 'VOL.');
  $volume = $issue = '';
  if ($has_vol) {
    $parts = upitt_islandora_digital_workflow_migration_explode_on_a_key(array("Vol.", "vol.", "VOL."), $title);
    if (count($parts) > 1) {
      @list($volume, $junk) = explode(" ", $parts[1]);
      $volume = trim($volume);
    }
  }
  $has_no = strstr($uctitle, 'NO.');
  if ($has_no) {
    $parts = upitt_islandora_digital_workflow_migration_explode_on_a_key(array("No.", "no.", "NO."), $title);
    if (count($parts) > 1) {
      $issue = trim($parts[1]);
    }
  }
  return array(
      'volume' => $volume,
      'issue' => $issue);
}

function upitt_islandora_digital_workflow_migration_explode_on_a_key($array_of_keys, $text) {
  $parts = array();
  foreach ($array_of_keys as $key) {
    $try_parts = explode($key, $text);
    if (count($try_parts) > 1) {
      $parts = $try_parts;
    }
  }
  return $parts;
}
