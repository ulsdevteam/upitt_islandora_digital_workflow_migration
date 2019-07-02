<div class="<?php print $class; ?>">
    <h3><?php print $title; ?></h3>
<?php foreach ($row as $key => $value): ?>
    <b><?php print $key; ?>:</b> <?php print (is_null($value) ? '<i>NULL</i>' : $value); ?><br />
<?php endforeach; ?>
</div>
<?php if ($extra_content): ?>
  <?php print $extra_content; ?>
  <br class="clearfloat" />
<?php endif; ?>