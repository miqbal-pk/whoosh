<?php

/**
 * @file
 * Model Platform Booking - Helper functions.
 */

/**
 * Create new BAT Unit.
 */
function _mp_booking_unit_create($name, $uid) {
  $unit = bat_unit_create(array(
    'type' => MP_BOOKING_UNIT_TYPE,
  ));
  $unit->created = REQUEST_TIME;
  $unit->type_id = MP_BOOKING_UNIT_TYPE_ID;
  $unit->default_state = 1;
  $unit->name = $name;
  $unit->uid = $uid;
  $unit->save();

  return $unit;
}

/**
 * Update BAT Unit.
 */
function _mp_booking_unit_update($unit, $name, $uid) {
  $unit->name = $name;
  $unit->uid = $uid;
  $unit->save();

  return $unit;
}

/**
 * Get Entity BAT Unit.
 */
function _mp_booking_get_unit($entity_type, $entity) {
  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  $unit = $wrapper->{MP_BOOKING_UNIT_FIELD_NAME}->value();

  return $unit;
}

/**
 * Set BAT Unit to the Entity.
 */
function _mp_booking_set_unit($entity_type, $entity, $unit) {
  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  $wrapper->{MP_BOOKING_UNIT_FIELD_NAME}->set($unit);
  $wrapper->save();
}

/**
 * Get all customer Jobs.
 */
function _mp_booking_get_all_customer_jobs($customer_uid, $_fields = array('nid')) {
  $query = db_select('node', 'n');
  $query->condition('type', 'job');
  $query->condition('uid', $customer_uid);
  $query->innerJoin('field_data_field_job_start_date', 'sd',
    'sd.entity_type = \'node\' AND sd.entity_id = n.nid');
  $query->condition('sd.field_job_start_date_value',
    date('Y-m-d H:i:s', REQUEST_TIME), '>');
  $query->fields('n', $_fields);
  $result = $query->execute()->fetchAllKeyed(0);

  return $result;
}

/**
 * Get opened customer jobs.
 */
function _mp_booking_get_opened_customer_jobs($customer_uid) {
  $customer_jobs = _mp_booking_get_all_customer_jobs($customer_uid, array('nid', 'title'));
  // Hide confirmed jobs.
  foreach ($customer_jobs as $_nid => $_job_title) {
    if (_modelplatform_job_is_confirmed($_nid)) {
      unset($customer_jobs[$_nid]);
    }
  }

  return $customer_jobs;
}

/**
 * Get auction event data.
 */
function _mp_booking_get_auction_event_data($model_uid, $customer_uid) {
  $result = NULL;

  // BAT Event.
  $event_query = db_select('bat_events', 'be');
  $event_query->condition('be.start_date', date('Y-m-d H:i:s', REQUEST_TIME), '>');

  // State.
  $event_query->innerJoin('field_data_event_state_reference', 'es',
    'es.entity_id = be.event_id AND es.entity_type = \'bat_event\' AND es.event_state_reference_state_id IN (3, 4, 5)');

  // Job.
  $event_query->innerJoin('field_data_field_job', 'fj',
    'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $event_query->innerJoin('node', 'n',
    'n.nid = fj.field_job_target_id');
  $event_query->condition('n.uid', $customer_uid);

  // BAT Unit.
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'febu',
    'febu.entity_id = be.event_id AND febu.entity_type = \'bat_event\'');
  $event_query->innerJoin('field_data_field_bat_unit', 'fbu',
    'fbu.field_bat_unit_target_id = febu.event_bat_unit_reference_target_id AND fbu.entity_type = \'user\'');
  $event_query->condition('fbu.entity_id', $model_uid);
  $event_query->condition('n.nid', arg(4));
  $event_query->addField('n', 'nid', 'job_nid');
  $event_query->addField('n', 'title', 'job_title');
  $event_query->addField('be', 'event_id', 'event_id');

  $query_result = $event_query->execute()->fetch();
  if (!empty($query_result)) {
    $result = $query_result;
  }

  return $result;
}

/**
 * Get auction event data for all auction
 */
function _mp_booking_get_auction_event_data_2($model_uid, $customer_uid) {
  $result = NULL;

  // BAT Event.
  $event_query = db_select('bat_events', 'be');
  $event_query->condition('be.start_date', date('Y-m-d H:i:s', REQUEST_TIME), '>');

  // State.
  $event_query->innerJoin('field_data_event_state_reference', 'es',
    'es.entity_id = be.event_id AND es.entity_type = \'bat_event\' AND es.event_state_reference_state_id IN (3, 4, 5)');

  // Job.
  $event_query->innerJoin('field_data_field_job', 'fj',
    'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $event_query->innerJoin('node', 'n',
    'n.nid = fj.field_job_target_id');
  $event_query->condition('n.uid', $customer_uid);

  // BAT Unit.
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'febu',
    'febu.entity_id = be.event_id AND febu.entity_type = \'bat_event\'');
  $event_query->innerJoin('field_data_field_bat_unit', 'fbu',
    'fbu.field_bat_unit_target_id = febu.event_bat_unit_reference_target_id AND fbu.entity_type = \'user\'');
  $event_query->condition('fbu.entity_id', $model_uid);
  $event_query->addField('n', 'nid', 'job_nid');
  $event_query->addField('n', 'title', 'job_title');
  $event_query->addField('be', 'event_id', 'event_id');

  $query_result = $event_query->execute()->fetchAll();
  if (!empty($query_result)) {
    $result = $query_result;
  }

  return $result;
}
/**
 * Get job auction.
 */
function _mp_booking_get_job_auction($model_uid, $customer_uid) {
  $result = NULL;

  // Auction.
  $event_query = db_select('mp_auction', 'a');

  // BAT Event.
  $event_query->innerJoin('field_data_field_auction_event', 'fae',
    'fae.entity_id = a.mp_auction_id AND fae.entity_type = \'mp_auction\'');
  $event_query->innerJoin('bat_events', 'be',
    'be.event_id = fae.field_auction_event_target_id');
  $event_query->innerJoin('field_data_field_job', 'fj', 'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $event_query->innerJoin('node', 'n', 'n.nid = fj.field_job_target_id');
  $event_query->condition('n.uid', $customer_uid);

  // BAT Unit.
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'febu',
    'febu.entity_id = be.event_id AND febu.entity_type = \'bat_event\'');
  $event_query->innerJoin('field_data_field_bat_unit', 'fbu',
    'fbu.field_bat_unit_target_id = febu.event_bat_unit_reference_target_id AND fbu.entity_type = \'user\'');
  $event_query->condition('fbu.entity_id', $model_uid);
  $event_query->addField('n', 'nid', 'job_nid');
  $event_query->addField('n', 'title', 'job_title');

  $query_result = $event_query->execute()->fetch();
  if (isset($query_result->job_nid)) {
    $result = node_load($query_result->job_nid);
  }

  return $result;
}

/**
 * Get model events.
 */
function _mp_booking_get_model_events($model_uid, $start_date, $end_date) {
  // BAT Event.
  $event_query = db_select('bat_events', 'be');

  // Job.
  $event_query->innerJoin('field_data_field_job', 'fj',
    'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $event_query->innerJoin('node', 'n', 'n.nid = fj.field_job_target_id');

  $or = db_or();

  if ($start_date) {
    // Get date values.
    $c_start_date = db_and();
    $c_start_date->condition('be.start_date', $start_date, '<=');
    $c_start_date->condition('be.end_date', $start_date, '>=');
    $or->condition($c_start_date);
  }

  if ($end_date) {
    $c_end_date = db_and();
    $c_end_date->condition('be.start_date', $end_date, '<=');
    $c_end_date->condition('be.end_date', $end_date, '>=');
    $or->condition($c_end_date);
  }

  if ($start_date && $end_date) {
    $c_into_date = db_and();
    $c_into_date->condition('be.start_date', $start_date, '>=');
    $c_into_date->condition('be.end_date', $end_date, '<=');
    $or->condition($c_into_date);
  }

  $event_query->condition($or);

  // BAT Unit.
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'febu',
    'febu.entity_id = be.event_id AND febu.entity_type = \'bat_event\'');

  $event_query->innerJoin('field_data_field_bat_unit', 'fbu',
    'fbu.field_bat_unit_target_id = febu.event_bat_unit_reference_target_id AND fbu.entity_type = \'user\'');
  $event_query->condition('fbu.entity_id', $model_uid);

  $event_query->addField('n', 'nid', 'job_nid');
  $event_query->addField('be', 'event_id', 'event_id');

  $event_query->orderBy('be.created', 'ASC');

  $result = $event_query->execute()->fetchAllKeyed(0);

  return $result;
}

/**
 * Get model auctions.
 */
function _mp_booking_get_model_auctions($model_uid, $start_date, $end_date) {
  $event_query = db_select('mp_auction', 'a');

  // BAT Event.
  $event_query->innerJoin('field_data_field_auction_event', 'fae',
    'fae.entity_id = a.mp_auction_id AND fae.entity_type = \'mp_auction\'');
  $event_query->innerJoin('bat_events', 'be',
    'be.event_id = fae.field_auction_event_target_id');

  // Job.
  $event_query->innerJoin('field_data_field_job', 'fj',
    'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $event_query->innerJoin('node', 'n', 'n.nid = fj.field_job_target_id');

  $or = db_or();

  if ($start_date) {
    // Get date values.
    $c_start_date = db_and();
    $c_start_date->condition('be.start_date', $start_date, '<=');
    $c_start_date->condition('be.end_date', $start_date, '>=');
    $or->condition($c_start_date);
  }

  if ($end_date) {
    $c_end_date = db_and();
    $c_end_date->condition('be.start_date', $end_date, '<=');
    $c_end_date->condition('be.end_date', $end_date, '>=');
    $or->condition($c_end_date);
  }

  if ($start_date && $end_date) {
    $c_into_date = db_and();
    $c_into_date->condition('be.start_date', $start_date, '>=');
    $c_into_date->condition('be.end_date', $end_date, '<=');
    $or->condition($c_into_date);
  }

  $event_query->condition($or);

  // BAT Unit.
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'febu',
    'febu.entity_id = fae.field_auction_event_target_id AND febu.entity_type = \'bat_event\'');

  $event_query->innerJoin('field_data_field_bat_unit', 'fbu',
    'fbu.field_bat_unit_target_id = febu.event_bat_unit_reference_target_id AND fbu.entity_type = \'user\'');
  $event_query->condition('fbu.entity_id', $model_uid);

  $event_query->addField('n', 'nid', 'job_nid');
  $event_query->addField('a', 'mp_auction_id', 'mp_auction_id');

  $event_query->orderBy('a.created', 'ASC');

  $result = $event_query->execute()->fetchAllKeyed(0);

  return $result;
}

/**
 * Get BAT Events of the BAT Unit.
 */
function _mp_booking_get_unit_events($unit, $start_date = NULL, $end_date = NULL, $blocking = NULL, $return_events = TRUE) {
  $or = db_or();

  $flag = FALSE;
  if ($start_date) {
    // Get date values.
    $c_start_date = db_and();
    $c_start_date->condition('be.start_date', $start_date, '<=');
    $c_start_date->condition('be.end_date', $start_date, '>=');
    $or->condition($c_start_date);
    $flag = TRUE;
  }

  if ($end_date) {
    $c_end_date = db_and();
    $c_end_date->condition('be.start_date', $end_date, '<=');
    $c_end_date->condition('be.end_date', $end_date, '>=');
    $or->condition($c_end_date);
    $flag = TRUE;
  }

  if ($start_date && $end_date) {
    $c_into_date = db_and();
    $c_into_date->condition('be.start_date', $start_date, '>=');
    $c_into_date->condition('be.end_date', $end_date, '<=');
    $or->condition($c_into_date);
    $flag = TRUE;
  }

  // Get blocking events.
  $event_query = db_select('bat_events', 'be');
  if ($flag) {
    $event_query->condition($or);
  }
  $event_query->innerJoin('field_data_event_state_reference', 'besr',
    'besr.entity_id = be.event_id AND besr.entity_type = \'bat_event\'');
  $event_query->innerJoin('field_data_event_bat_unit_reference', 'beur',
    'beur.entity_id = be.event_id '
    . ' AND beur.entity_type = \'bat_event\''
    . ' AND beur.event_bat_unit_reference_target_id = ' . $unit->unit_id );
  $event_query->innerJoin('bat_event_state', 'bes',
    'bes.id = besr.event_state_reference_state_id');
  if (isset($blocking)) {
    $event_query->condition('bes.blocking', $blocking);
  }
  $event_query->addField('be', 'event_id', 'event_id');
  $event_ids = $event_query->execute()->fetchCol(0);

  if ($return_events) {
    $result = entity_load('bat_event', $event_ids);
  }
  else {
    $result = $event_ids;
  }

  return $result;
}

/**
 * Get BAT Event auction ids.
 */
function mp_booking_get_event_auction_ids($event) {
  $query = db_select('field_data_field_auction_event', 'fae');
  $query->condition('field_auction_event_target_id', $event->event_id);
  $query->condition('entity_type', 'mp_auction');
  $query->fields('fae', array('entity_id'));

  $auction_ids = $query->execute()->fetchCol(0);

  return $auction_ids;
}

/**
 * Get BAT Event auction entities.
 */
function mp_booking_get_event_auctions($event) {
  $auction_ids = mp_booking_get_event_auction_ids($event);

  $auctions = entity_load('mp_auction', $auction_ids);

  return $auctions;
}

function _mp_booking_event_create($unit_id, $state_id, $uid, $job_nid = 0, $start_date = NULL, $end_date = NULL) {
  if (!$job = node_load($job_nid)) {
    if ($job_nid !== MP_BOOKING_EMPTY_JOB) {
      return;
    }
  }
  $event_params = array(
    'type' => MP_BOOKING_MODEL_AVAILABILITY,
  );
  $event = bat_event_create($event_params);
  $event->field_job[LANGUAGE_NONE][0]['target_id'] = $job_nid;
  $event->event_bat_unit_reference[LANGUAGE_NONE][0]['target_id'] = $unit_id;
  $event->event_state_reference[LANGUAGE_NONE][0]['state_id'] = $state_id;
  $event->field_job_cost = $job->field_job_daily_rate_from;
  if ($start_date) {
    $event->start_date = $start_date;
  }
  if ($end_date) {
    $event->end_date = $end_date;
  }
  $event->uid = $uid;

  return $event;
}

function _mp_booking_get_calendar_account() {
  global $user;

  $args = arg();

  $account = NULL;
  if (isset($args[3])
    && $args[0] === 'job'
    && $args[1] === 'model'
    && $args[3] === 'invite') {
    $account = user_load(intval($args[2]));
  }
  elseif (!isset($args[1])
    && isset($args[0])
    && $args[0] == 'user'
    && $user->uid) {
    $account = user_load($user->uid);
  }
  elseif (!isset($args[2])
    && isset($args[1])
    && intval($args[1])
    && $args[0] === 'user') {
    $account = user_load(intval($args[1]));
    if (!isset($account->roles[5])) {
      $account = NULL;
    }
  }
  elseif (isset($args[3])
    && $args[0] === 'job'
    && $args[1] === 'event'
    && $args[3] === 'auction') {
    if ($event = bat_event_load(intval($args[2]))) {
      $query = db_select('field_data_event_bat_unit_reference', 'b_unit');
      $query->condition('b_unit.entity_type', 'bat_event');
      $query->condition('b_unit.entity_id', $event->event_id);
      $query->innerJoin('field_data_field_bat_unit', 'u_unit',
        'u_unit.field_bat_unit_target_id = b_unit.event_bat_unit_reference_target_id');
      $query->condition('u_unit.entity_type', 'user');
      $query->addField('u_unit', 'entity_id', 'uid');
      $uid = $query->execute()->fetchField();

      $account = user_load($uid);
    }
  }

  return $account;
}

function _mp_booking_get_calendar_busy_access() {
  global $user;

  $args = arg();

  $access = FALSE;
  if (isset($args[3])
    && $args[0] === 'job'
    && $args[1] === 'model'
    && intval($args[2]) === intval($user->uid)) {
    $access = TRUE;
  }
  elseif (isset($args[0]) && $args[0] == 'user') {
    $access = TRUE;
  }
  elseif (isset($args[3])
    && $args[0] === 'job'
    && $args[1] === 'event'
    && intval($args[2])
    && $args[3] === 'auction') {
    $access = FALSE;
  }

  return $access;
}

/**
 * Generate path for calendar events data.
 */
function _rcb_generate_calendar_events_path($unit) {
  $events_path_params = array(
    'q=bat/v2/events-calendar',
    'unit_types=' . $unit->type_id,
    'event_types=' . 'model_availability',
    'unit_ids=' . $unit->unit_id,
  );
  $events_path = '?' . implode('&', $events_path_params);

  return $events_path;
}

/**
 * Page callback.
 */
function mp_booking_model_propose($nid) {
  global $user;

  if (!isset($user->roles[5])) {
    drupal_set_message(t('Only model can send propose to the customer.'), 'warning');

    return '';
  }

  $job = node_load($nid);

  // Job dates check.
  if (isset($job->field_job_start_date[LANGUAGE_NONE][0]['value'])) {
    $start_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value'];
  }
  else {
    return;
  }
  if (isset($job->field_job_start_date[LANGUAGE_NONE][0]['value2'])) {
    $end_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value2'];
  }
  else {
    return;
  }

  // Get model unit.
  $model_account = user_load($user->uid);
  $unit = _mp_booking_get_unit('user', $model_account);

  // Create event.
  $event = _mp_booking_event_create($unit->unit_id, MP_BOOKING_STATE_AUCTION, $user->uid, $nid, $start_date, $end_date);
  bat_event_save($event);

  drupal_goto('job/event/' . $event->event_id . '/auction');
}

function mp_booking_job_model_search($nid) {
  $goto_front = FALSE;

  if ($job = node_load($nid)) {
    // Job dates check.
    if (isset($job->field_job_start_date[LANGUAGE_NONE][0]['value'])) {
      $start_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value'];
      $start_date_array = explode(' ', $start_date);
      $start_date = $start_date_array[0];
    }
    else {
      $goto_front = TRUE;
    }
    if (isset($job->field_job_start_date[LANGUAGE_NONE][0]['value2'])) {
      $end_date = $job->field_job_start_date[LANGUAGE_NONE][0]['value2'];
      $end_date_array = explode(' ', $end_date);
      $end_date = $end_date_array[0];
    }
    else {
      $goto_front = TRUE;
    }
  }
  else {
    $goto_front = TRUE;
  }

  if ($goto_front) {
    drupal_goto('<front>');
  }
  else {
    $query = array(
      'start_date' => array(
        'date' => $start_date,
      ),
      'end_date' => array(
        'date' => $end_date,
      ),
      'daily_rate' => $job->field_job_daily_rate_to[LANGUAGE_NONE][0]['value'],
    );
    drupal_goto('models', array('query' => $query));
  }
}

function mp_booking_get_event_last_auction($event_id) {
  $event_query = db_select('mp_auction', 'a');

  // BAT Event.
  $event_query->innerJoin('field_data_field_auction_event', 'fae',
    'fae.entity_id = a.mp_auction_id AND fae.entity_type = \'mp_auction\'');
  $event_query->condition('fae.field_auction_event_target_id', $event_id);

  $event_query->addField('a', 'mp_auction_id', 'auction_id');
  $event_query->orderBy('a.created', 'desc');

  return $event_query->execute()->fetch();
}

/**
 * Delete other job events.
 */
function _mp_booking_cancel_other_events($job_nid, $excluded_event_id) {
  $event_ids = _mp_booking_get_job_events($job_nid);
  unset($event_ids[$excluded_event_id]);
  bat_event_delete_multiple($event_ids);
}

/**
 * Get all job event IDs.
 */
function _mp_booking_get_job_events($job_nid) {
  $query = db_select('bat_events', 'be');
  $query->innerJoin('field_data_field_job', 'fj',
    'fj.entity_id = be.event_id AND fj.entity_type = \'bat_event\'');
  $query->condition('fj.field_job_target_id', $job_nid);
  $query->addField('be', 'event_id', 'event_id');
  $event_ids = $query->execute()->fetchAllKeyed(0, 0);

  return $event_ids;
}

/**
 * Get all event auction IDs.
 */
function _mp_booking_get_event_auctions($event_id) {
  $query = db_select('mp_auction', 'a');
  $query->innerJoin('field_data_field_auction_event', 'fe',
    'fe.entity_id = a.mp_auction_id AND fe.entity_type = \'mp_auction\'');
  $query->condition('fe.field_auction_event_target_id', $event_id);
  $query->addField('a', 'mp_auction_id', 'mp_auction_id');
  $mp_auction_ids = $query->execute()->fetchAllKeyed(0, 0);

  return $mp_auction_ids;
}

function _mp_booking_get_auctioned_job($model_uid, $customer_uid) {
  $event_data = _mp_booking_get_auction_event_data($model_uid, $customer_uid);
  $auctioned_job =null;
  if($event_data) {
    $auctioned_job = node_load($event_data->job_nid);
  }
  
  return $auctioned_job;
}

function mp_booking_reciept_approve($event, $item_id) {
  global $user;

  $event_wrapper = entity_metadata_wrapper('bat_event', $event);
  $fc_items = $event_wrapper->field_additional_receipts->value();
  foreach ($fc_items as $fc_item) {
    if ($fc_item->item_id == $item_id) {
      $fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
      $fc_item_wrapper->field_reciept_approved_by_admin->set(1);
      $fc_item_wrapper->field_reciept_admin->set($user);
      $fc_item_wrapper->save();

      // It will call entity_update and then order will be updated.
      bat_event_save($event);
    }
  }

  drupal_goto('admin/modelplatform/invoice_reciepts');
}

function mp_booking_reciept_remove($event, $item_id) {
  global $user;

  $event_wrapper = entity_metadata_wrapper('bat_event', $event);
  $fc_items = $event_wrapper->field_additional_receipts->value();
  foreach ($fc_items as $fc_item) {
    if ($fc_item->item_id == $item_id) {
      $fc_item_wrapper = entity_metadata_wrapper('field_collection_item', $fc_item);
      $fc_item_wrapper->field_reciept_approved_by_admin->set(0);
      $fc_item_wrapper->field_reciept_admin->set($user);
      $fc_item_wrapper->save();

      // It will call entity_update and then order will be updated.
      bat_event_save($event);
    }
  }

  // It will call entity_update and then order will be updated.
  bat_event_save($event);

  drupal_goto('admin/modelplatform/invoice_reciepts');
}