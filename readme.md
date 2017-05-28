# PHPSrbija 2017 - PHP and IoT - Creating a Deployment device

Hi! If you're reading this, you're probably sitting at our workshop - PHP and IoT - Creating a Deployment device. This is a guide to help you setup your Raspberry Pi. Enjoy!

## Requirements

First of, you'll need to make sure you have the following:

- A Laptop (duh!)
- Git installed on your machine
- A GitHub account
- Composer

If you don't have something from the above list, take a minute and install it.

## WiFi

Make sure that you are connected to the network **"PHPSrb2017booth"**. All Raspberries and NodeMCUs are on this network, and you need to be too in order to SSH into them.

The WiFi password is up on the slides!

## Create a GitHub repo

You should create a repo on GitHub where the code for this test deployment will live. Create it and name it **"phpiot"**.

## Create a Laravel application

Now we will create a Laravel application by typing this into your console: (you do have Composer installed, right?)

```sh
$ composer create-project --prefer-dist laravel/laravel phpsrb
$ cd phpsrb
```

Now we want to push that project you just created to your GitHub repository.

```sh
$ git init

$ git add .

$ git remote add origin https://github.com/{username}/phpiot.git

$ git commit -m ‘Initial commit’

$ git push -u origin master
```

Okay! Now you're ready to connect with the Raspberry Pi!

## Connecting with Raspberry Pi

Take the provided USB-MicroUSB cable and connect your computer and the Raspberry Pi. That cable will serve as Raspberry's power source, so don't unplug it. Wait a minute for it to boot up.

Now, look on the back side of your Raspberry Pi. You will see a number. You can use that number to SSH into the Raspberry like this:

```sh
$ ssh pi@jenkinsX.local
```

Where X is the number of your Pi. The password? **jenkinsX**.

## Install Jenkins on Raspberry Pi

Okay, now we've got to the real stuff.

While logged into your Pi, execute these commands:

```sh
# make sure Java 8 is selected here. If not, select it.
$ sudo update-alternatives --config java 

$ mkdir jenkins

$ cd jenkins

$ wget -q -O - https://jenkins-ci.org/debian/jenkins-ci.org.key | sudo apt-key add -

$ sudo sh -c 'echo deb http://pkg.jenkins-ci.org/debian binary/ > /etc/apt/sources.list.d/jenkins.list'

$ sudo apt-get update

$ sudo apt-get install jenkins

$ sudo /etc/init.d/jenkins start
```

After the Jenkins is started, you can access it by typing this URL into your browser:

```sh
http://jenkinsX.local:8080
```

Again, X is the number of your Pi.

You will be greated by the "Unlock Jenkins" screen. Do what is says, find the password by typing

```sh
cat /var/lib/jenkins/secrets/initialAdminPassword
```

and then copy it into the **Administrator Password** field in the browser.

During the installation, Jenkins will ask you a few things. Make sure you select **"Install suggested plugins"** and create a user with the following username and password:

```sh
Username: admin
Password: jenkins
```

It's important that you give it exactly this username and password, the rest of the workshop depends on it!

Okay, now you can login to Jenkins!

## Setup a Jenkins job

Create a new Jenkins job.

Give it a name of **buildpi** and select **"Freestyle Job"**. Again, make sure you get the name of the job exactly right.

Now we will setup the Jenkins job. Open it and give it the following options:

1. Under General, check the **GitHub Project** box.
2. Set **https://github.com/{username}/phpiot/** as the project URL.
3. Under Source Code Management, check **"Git"** box.
4. Set **https://github.com/{username}/phpiot.git** as the Repository URL.

## Setup Jenkins credentials

When you checked the **"Git"** box under Source Code Management, a Credentials select box has appeared. You need to **"Add new Credentials"**. You will be asked for your GitHub login. Enter your login email and password and click okay. If you don't have a GitHub account, you can use this one:

```
Username: phpiot
Password: phpiot2017
```

Now that's all sorted out, we get to configure what our job will do.

First thing you need to do - under **"Build Triggers"**, check the **"Trigger Builds Remotely"**.

A field for **"Authentication Token"** will appear. You need to enter this:

```
php-srb-2017
```

It's important that you type it exactly like that!

## Build steps

Under **"Build Steps"**, add a Build Step of type **"Execute Shell"**. Copy this script there: (and make sure you change the username placeholder to your username!)

```sh
#!/bin/bash
sudo -s /bin/bash -c 'gtts-cli "Build started!" | mpg123 -' pi
sudo rm -rf phpiot
git clone https://github.com/{username}/phpiot.git ./phpiot
cd phpiot
cp .env.example .env
composer install
php artisan key:generate
vendor/bin/phpunit
if [[ $? -eq 0 ]]
then
  sudo -s /bin/bash -c 'gtts-cli "The build was successful!" | mpg123 -' pi
  sudo python /home/pi/green-led.py
  exit 0
else
  sudo -s /bin/bash -c 'gtts-cli "The build has failed." | mpg123 -' pi
  sudo python /home/pi/red-led.py
  exit 1
fi
```

Save the Job, and now you can try it out by waving left over your NodeMCU.

Wait for a few minutes, and see the magic happen! Cool, right?

## Change a PHPUnit test

Now, we would like to change a test to make our build fail. Open the ExampleTest in your code editor 

```
tests/Feature/ExampleTest.php
```

and change it so it looks like this:

```php
class ExampleTest extends TestCase
{
   /**
    * A basic test example.
    *
    * @return void
    */
   public function testBasicTest()
   {
       $response = $this->get('/');

       $response->assertStatus(418); // I’m a teapot
   }
}
```

Now you can wave left over the NodeMCU and the build will start!

## Thanks!