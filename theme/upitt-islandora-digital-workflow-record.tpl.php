<div class="<?php print $class; ?>">
    <h3><?php print $title; ?></h3>
<?php foreach ($row as $key => $value): ?>
    <b><?php print $key; ?>:</b> <?php print $value; ?><br />
<?php endforeach; ?>
  <?php if ($extra_content): ?>
    <?php print $extra_content; ?>
    <br class="clearfloat" />
  <?php endif; ?>
</div>
