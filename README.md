# php-disable-eval

A PHP extension to disable `eval()` and `create_function()`.

## Installation

```bash
phpize
./configure
make
sudo make install
```

Then add

```
extension=disableeval.so
```

to your `php.ini`.

## Configuration Options (php.ini)

  * `disableeval.enabled` (boolean, default `On`): whether to enable the extension.
  * `disableeval.disallow_create_func` (boolean, default `On`): whether to disallow `create_function()`. Works in PHP 7 onle, because `create_function` was removed from PHP 8.
  * `disableeval.mode` (integer, default 1): what to do on `eval()` or `create_function()`:
    * 0 (`DISABLEEVAL_MODE_IGNORE`): do nothing, continue normal execution;
    * 1 (`DISABLEEVAL_MODE_THROW`): throw an `ErrorException` exception;
    * 2 (`DISABLEEVAL_MODE_WARN`): display a warning;
    * 3 (`DISABLEEVAL_MODE_SCREAM`): display a warning that cannot be suppressed with the `@` operator;
    * 4 (`DISABLEEVAL_MODE_BAILOUT`): emit a fatal error and abort the script.
 
 For security reasons, all options are [`PHP_INI_SYSTEM`](https://www.php.net/manual/en/configuration.changes.modes.php): that is, they can only be set in `php.ini` or `httpd.conf`.
