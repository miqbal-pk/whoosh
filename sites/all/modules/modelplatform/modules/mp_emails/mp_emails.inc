<?php

/**
 * Created by PhpStorm.
 * User: cimpleo
 * Date: 14.08.17
 * Time: 14:49
 */

/**
 * Prepare email params by event.
 */
function _mp_emails_get_event_params($event) {
  global $base_url;

  $event_wrapper = entity_metadata_wrapper('bat_event', $event);

  $unit = $event_wrapper->event_bat_unit_reference->value();
  $job = $event_wrapper->field_job->value();
  $job_wrapper = entity_metadata_wrapper('node', $job);

  $model_uid = $unit->uid;
  $model = user_load($model_uid);
  $model_wrapper = entity_metadata_wrapper('user', $model);
  $customer = user_load($job->uid);
  $customer_wrapper = entity_metadata_wrapper('user', $customer);

  $model_profiles = profile2_load_by_user($model);
  $model_basic_information = entity_metadata_wrapper('profile2', $model_profiles['model_basic_informaiton']);

  $tid = $model_basic_information->field_model_gender->tid->value();
  if ($tid == 89) {
    $model_salutation = array(
      'en' => 'Dear',
      'de' => 'Liebe',
    );
    $customer_salutation = array(
      'en' => 'Dear Mrs',
      'de' => 'Lieber Frau',
    );
  }
  else {
    $model_salutation = array(
      'en' => 'Dear',
      'de' => 'Lieber',
    );
    $customer_salutation = array(
      'en' => 'Dear Mr.',
      'de' => 'Lieber Herr',
    );
  }

  $job_date_format = 'Y-m-d H:i';
  $job_start_date = date($job_date_format, $job_wrapper->field_job_start_date->value->value());
  $job_end_date = date($job_date_format, $job_wrapper->field_job_start_date->value2->value());
  $job_date = "$job_start_date - $job_end_date";
  $job_daily_rate_from = commerce_currency_format($job_wrapper->field_job_daily_rate_from->amount->value(), $job_wrapper->field_job_daily_rate_from->currency_code->value());
  $job_daily_rate_to = commerce_currency_format($job_wrapper->field_job_daily_rate_to->amount->value(), $job_wrapper->field_job_daily_rate_to->currency_code->value());
  $job_daily_rate = "$job_daily_rate_from - $job_daily_rate_to";

  array(
    '@model_full_name' => $model_wrapper->field_full_name->value(),
    '@model_language' => $model->language,
    '@model_mail' => $model->mail,
    '@customer_full_name' => $customer_wrapper->field_full_name->value(),
    '@customer_language' => $customer->language,
    '@customer_mail' => $model->mail,
    '@model_salutation' => $model_salutation[$model->language],
    '@customer_salutation' => $customer_salutation[$customer->language],
    '@job_title' => $job->title,
    '!job_description' => $job_wrapper->body->value->value(),
    '@job_date' => $job_date,
    '@job_daily_rate' => $job_daily_rate,
    '@url' => $base_url . '/job/event/' . $event->event_id . '/auction',
  );
}